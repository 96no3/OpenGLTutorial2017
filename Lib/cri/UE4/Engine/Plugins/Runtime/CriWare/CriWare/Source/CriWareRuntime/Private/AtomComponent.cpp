/****************************************************************************
 *
 * CRI Middleware SDK
 *
 * Copyright (c) 2013-2018 CRI Middleware Co., Ltd.
 *
 * Library  : CRIWARE plugin for Unreal Engine 4
 * Module   : AudioComponent for Atom
 * File     : AtomComponent.cpp
 *
 ****************************************************************************/

/***************************************************************************
 *      インクルードファイル
 *      Include files
 ***************************************************************************/
/* モジュールヘッダ */
#include "AtomComponent.h"

/* CRIWAREプラグインヘッダ */
#include "CriWareRuntimePrivatePCH.h"
#include "AtomSoundObject.h"
#include "AtomActiveSound.h"
#include "AtomPlaylist.h"

/* Unreal Engine 4関連ヘッダ */
#include "Engine/Engine.h"
#include "Application/ThrottleManager.h"
#include "AudioThread.h"
#include "UObject/Package.h"
#include "Misc/PackageName.h"
#include "Engine/LocalPlayer.h"
#include "UnrealClient.h"
#if WITH_EDITOR
#include "DrawDebugHelpers.h"
#endif

/***************************************************************************
 *      定数マクロ
 *      Macro Constants
 ***************************************************************************/
#define LOCTEXT_NAMESPACE "AtomComponent"

/***************************************************************************
 *      処理マクロ
 *      Macro Functions
 ***************************************************************************/

/***************************************************************************
 *      データ型宣言
 *      Data Type Declarations
 ***************************************************************************/

/***************************************************************************
 *      変数宣言
 *      Prototype Variables
 ***************************************************************************/

/***************************************************************************
 *      クラス宣言
 *      Prototype Classes
 ***************************************************************************/

/***************************************************************************
 *      関数宣言
 *      Prototype Functions
 ***************************************************************************/

/***************************************************************************
 *      変数定義
 *      Variable Definition
 ***************************************************************************/
int32 UAtomComponent::AtomComponentIDCounter = 0;
TMap<uint64, UAtomComponent*> UAtomComponent::AtomIDToComponentMap;
TArray<UAtomComponent*> UAtomComponent::AtomRootedComponentArray;

/***************************************************************************
 *      クラス定義
 *      Class Definition
 ***************************************************************************/
FAtomAisacControlParam::FAtomAisacControlParam()
{
#if WITH_EDITOR
	/* Localization of unreal properties metadata with LOCTEXT markups and reflection */
	CRI_LOCSTRUCT(StaticStruct());
#endif
}

FAtomSelectorParam::FAtomSelectorParam()
{
#if WITH_EDITOR
	/* Localization of unreal properties metadata with LOCTEXT markups and reflection */
	CRI_LOCSTRUCT(StaticStruct());
#endif
}

UAtomComponent::UAtomComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, AtomComponentID(0)
	, Filter_Frequency(MAX_FILTER_FREQUENCY)
{
#if WITH_EDITOR
	/* Localization of unreal properties metadata with LOCTEXT markups and reflection */
	CRI_LOCCLASS(GetClass());
#endif

	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	bTickInEditor = true;

	bAutoDestroy = false;
	bPersistAcrossLevelTransition = false;
	bAutoActivate = true;
	bIsUISound = false;
	bIsPreviewSound = false;
	bStopWhenOwnerDestroyed = true;
	DefaultVolume = 1.0f;
	bEnableMultipleSoundPlayback = false;
	DefaultSoundObject = nullptr;
	DefaultBlockIndex = 0;
	Player = NULL;
	Source = NULL;
	DistanceFactor = 1.0f;
	CueSheet = NULL;
	PlaybackId = 0xFFFFFFFF;

#if !defined(CRIWARE_UE4_LE)	/* <cri_delete_if_LE> */
#endif	/* </cri_delete_if_LE> */

	/* 音源位置更新のため、OnUpdateTransformを呼び出してもらうよう指定する。 */
	bWantsOnUpdateTransform = true;

	/* 不要な機能を無効化 */
	bNeverNeedsRenderUpdate = true;

#if WITH_EDITORONLY_DATA
	/* 音源のスピーカアイコン表示を有効にする */
	bVisualizeComponent = true;
#endif

#if WITH_EDITOR
	/* デリゲート登録のリセット */
	OnWorldCleanedUpHandle.Reset();
#endif

	CreatedTime = FPlatformTime::Seconds();

	/* Componentのタグ付け */
	this->ComponentTags.Add(TEXT("AtomComponent"));

	/* プロファイラで列挙するためComponentをMapに登録 */
	if (!HasAnyFlags(RF_ClassDefaultObject)) {
		AtomComponentID = ++AtomComponentIDCounter;
		AtomIDToComponentMap.Add(AtomComponentID, this);
	}
}

void UAtomComponent::MarkAsRooted()
{
	AddToRoot();
	AtomRootedComponentArray.Add(this);
	bPersistAcrossLevelTransition = true;
}

/* ルートオブジェクトのTick */
void UAtomComponent::TickRootedComponents()
{
	for (UAtomComponent* AtomComponent : AtomRootedComponentArray) {
		AtomComponent->OnTickComponent();
	}
}

void UAtomComponent::SetSound(USoundAtomCue* NewSound)
{
	/* キューシートの取得 */
	USoundAtomCueSheet* NewCueSheet = ((NewSound != nullptr) ? NewSound->CueSheet : nullptr);

	/* キュー名の取得 */
	FString NewCueName = ((NewSound != nullptr) ? NewSound->CueName : "");

	/* データの指定 */
	SetSound(NewCueSheet, NewCueName);

	/* サウンドデータの更新 */
	Sound = NewSound;
}

void UAtomComponent::SetSound(USoundAtomCueSheet* InCueSheet, FString InCueName)
{
	/* 再生中の音声を停止 */
	if (bEnableMultipleSoundPlayback == false) {
		Stop();
	}

	/* 既存サウンドデータのクリア */
	Sound = nullptr;

	/* キューシートの変更チェック */
	if (CueSheet == InCueSheet) {
		/* データが変更されていない場合はリロード不要 */
	} else {
		/* 既存キューシートを一旦破棄 */
		if (CueSheet != nullptr) {
			CueSheet->Release();
			CueSheet = nullptr;
		}

		/* キューシートのリロード */
		CueSheet = InCueSheet;
		if (CueSheet != nullptr) {
			CueSheet->AddRef();
		}
	}

	/* キュー名の保存 */
	CueName = InCueName;

	/* プレイリストのクリア */
	if (Playlist != nullptr) {
		Playlist->Reset();
	}
}

/* 音声データのセット */
void UAtomComponent::EnqueueSound(USoundAtomCue* NewSound)
{
	/* プレイリストに追加 */
	if (Playlist != nullptr) {
		Playlist->Add(NewSound);
	}
}

int32 UAtomComponent::GetNumQueuedSounds()
{
	if (Playlist == nullptr) {
		return 0;
	}

	/* 排他制御区間の開始 */
	Playlist->Lock();

	/* キューに残っているサウンドの数を取得 */
	int32 NumQueued = Playlist->Num();

	/* 再生位置の取得 */
	int32 Pos = Playlist->Tell();

	/* 排他制御区間の終了 */
	Playlist->Unlock();

	/* 再生済みの音声を除外 */
	return (NumQueued - Pos);
}

void UAtomComponent::Play(float StartTime)
{
	/* 既に再生中の場合は一旦停止 */
	if (bEnableMultipleSoundPlayback == false) {
		Stop();
	}

	/* 再生用のリソースが確保済みかどうかチェック */
	if (Player == NULL) {
		/* リソースの確保 */
		AllocateResource();

		/* 発音が可能かどうかチェック */
		if (Player == NULL) {
			/* プレーヤが存在しない場合は再生不可 */
			return;
		}
	}

#if WITH_EDITOR
	/* ゲーム実行中かどうかチェック */
	UWorld* World = GetWorld();
	bool bIsGameWorld = (World ? World->IsGameWorld() : false);

	/* 発音の可否を判定 */
	/* 備考）エディタ上では以下のケースのみ発音を行う。             */
	/* 　　　- プレビュー実行中。                                   */
	/* 　　　- （PersonaやMatineeで）UIサウンドとして発音する場合。 */
	if ((GIsEditor != false) && (bIsGameWorld == false) && (bIsUISound == false)) {
		return;
	}

	/* 既存デリゲート登録の解除 */
	if (OnWorldCleanedUpHandle.IsValid() != false) {
		FWorldDelegates::OnWorldCleanup.Remove(OnWorldCleanedUpHandle);
		OnWorldCleanedUpHandle.Reset();
	}

	/* プレビュー終了検知用にデリゲートを登録 */
	if (GEngine != nullptr) {
		OnWorldCleanedUpHandle = FWorldDelegates::OnWorldCleanup.AddUObject(this, &UAtomComponent::OnWorldCleanedUp);
	}
#endif

	/* キューシート、キュー名の補完 */
	if (Sound != nullptr) {
		/* キューシートのロード */
		/* 注意）マップ上に配置された音はSetSoundを経由しないため、 */
		/* 　　　この時点でCueSheetがnullptrの可能性あり。          */
		if (CueSheet == nullptr) {
			CueSheet = Sound->CueSheet;
			if (CueSheet != nullptr) {
				CueSheet->AddRef();
			}
		}

		/* 互換性維持のための処理 */
		/* 備考）2014/03以前のプラグインで作成したデータは、	*/
		/* 　　　CueSheetの参照ではなくCueSheetNameを使用。		*/
		if ((CueSheet == nullptr) && (Sound->CueSheetName.Len() > 0)) {
			FString PackagePath = FPackageName::GetLongPackagePath(Sound->GetOutermost()->GetName());
			CueSheet = USoundAtomCueSheet::Find(*Sound->CueSheetName, PackagePath);
			if (CueSheet != nullptr) {
				CueSheet->AddRef();
			}
		}

		/* キュー名の補完 */
		if (CueName.Len() <= 0) {
			CueName = Sound->CueName;
		}
	}

	FString SelectedCueName = CueName;
	USoundAtomCueSheet* SelectedCueSheet = CueSheet;

	/* キュー名がプレイリストから補完可能かチェック */
	if ((Playlist != nullptr) && (SelectedCueName.Len() <= 0)) {
		/* 排他制御区間の開始 */
		Playlist->Lock();

		/* 再生位置の取得 */
		int32 Pos = Playlist->Tell();

		/* リストアイテムの取得 */
		const FAtomPlaylistItem* Item = Playlist->Get(Pos);
		if ((Item != nullptr) && (Item->Sound != nullptr)) {
			/* キューの情報を取得 */
			SelectedCueName = Item->Sound->CueName;
			SelectedCueSheet = Item->Sound->CueSheet;

			/* 再生位置の更新 */
			Playlist->Seek(Pos + 1);
		}

		/* 排他制御区間の終了 */
		Playlist->Unlock();
	}

	/* キュー名が指定されていない場合は何もしない */
	if (SelectedCueName.Len() <= 0) {
		return;
	}

	/* キュー名の設定 */
	CriAtomExAcbHn AcbHn = ((SelectedCueSheet != nullptr) ? SelectedCueSheet->GetAcbHn() : nullptr);
	criAtomExPlayer_SetCueName(Player, AcbHn, TCHAR_TO_UTF8(*SelectedCueName));

	/* 再生パラメータの設定*/
	criAtomExPlayer_SetVolume(Player, DefaultVolume);
	criAtomExPlayer_SetFirstBlockIndex(Player, DefaultBlockIndex);
	criAtomExPlayer_SetStartTime(Player, (CriSint32)(StartTime * 1000.0f));
	for (int32 i = 0; i < DefaultAisacControl.Num(); i++) {
		const FAtomAisacControlParam& P = DefaultAisacControl[i];
		criAtomExPlayer_SetAisacByName(Player, TCHAR_TO_UTF8(*P.Name), P.Value);
	}
	for (int32 i = 0; i < DefaultSelectorLabel.Num(); i++) {
		const FAtomSelectorParam& P = DefaultSelectorLabel[i];
		criAtomExPlayer_SetSelectorLabel(Player, TCHAR_TO_UTF8(*P.Selector), TCHAR_TO_UTF8(*P.Label));
	}

	/* 音源の位置を更新 */
	UpdateTransform();


#if !defined(CRIWARE_UE4_LE)	/* <cri_delete_if_LE> */
#endif	/* </cri_delete_if_LE> */

	/* サウンドオブジェクトを設定 */
	SetSoundObject(DefaultSoundObject);

#if WITH_EDITOR
	/* エディター上（非プレビュー中）のみ鳴らす音はパン3Dで再生 */
	/* 注意）シーケンサーで再生する音は常にUIサウンドとして再生されるため、	*/
	/* 　　　UIサウンドであっても暗黙でパン3Dを適用してはいけない。			*/
	if ((bIsGameWorld == false) && (bIsUISound == true)) {
		criAtomExPlayer_SetPanType(Player, CRIATOMEX_PAN_TYPE_PAN3D);
	}
#endif

	/* 再生の開始 */
	PlaybackId = criAtomExPlayer_Start(Player);

	/* 再生の開始を通知 */
	bIsActive = true;

#if WITH_EDITOR
	/* Play In Editorモードで減衰距離を表示するため、キュー情報から取得 */
	if (GIsPlayInEditorWorld && MaxDistanceSphereComponent && MinDistanceSphereComponent) {
		CriAtomExCueInfo cueInfo = { 0 };
		CriBool isGetCueInfoSucceed = CRI_FALSE;
		if (criAtomExAcb_ExistsName(static_cast<CriAtomExAcbHn>(CueSheet->GetAcbHn()), TCHAR_TO_UTF8(*(CueName))) == CRI_TRUE) {
			isGetCueInfoSucceed = criAtomExAcb_GetCueInfoByName(static_cast<CriAtomExAcbHn>(CueSheet->GetAcbHn()), TCHAR_TO_UTF8(*(CueName)), &cueInfo);
		}

		MaxDistanceSphereComponent->SetSphereRadius(cueInfo.pos3d_info.max_distance / DistanceFactor, false);
		MinDistanceSphereComponent->SetSphereRadius(cueInfo.pos3d_info.min_distance / DistanceFactor, false);
	}
#endif
}

/* データ要求コールバック */
void CRIAPI UAtomComponent::OnDataRequest(
	void *Object, CriAtomExPlaybackId PlaybackId, CriAtomPlayerHn Player)
{
	/* プレイリストの有無をチェック */
	FAtomPlaylist* Playlist = reinterpret_cast<FAtomPlaylist*>(Object);
	if (Playlist == nullptr) {
		return;
	}

	/* 排他制御区間の開始 */
	Playlist->Lock();

	/* 再生位置の取得 */
	int32 Pos = Playlist->Tell();

	/* リストアイテムの取得 */
	const FAtomPlaylistItem* Item = Playlist->Get(Pos);
	if ((Item != nullptr) && (Item->AwbHn != nullptr)) {
		/* 連結する波形データをセット */
		criAtomPlayer_SetWaveId(Player, Item->AwbHn, Item->WaveId);

		/* 再生位置の更新 */
		Playlist->Seek(Pos + 1);
	}

	/* 排他制御区間の終了 */
	Playlist->Unlock();
}

void UAtomComponent::Stop()
{
	/* 発音中かどうかチェック */
	if (bIsActive == false) {
		return;
	}

	if (Player == NULL) {
		bIsActive = false;
		return;
	}

	/* 再生を停止 */
	/* 注意）GC処理の順序によっては、ライブラリ終了処理後にここに来る可能性がある。 */
	if (criAtomEx_IsInitialized() != CRI_FALSE) {
		criAtomExPlayer_Stop(Player);
	}

	/* 再生の終了を通知 */
	bIsActive = false;
}

EAtomComponentStatus UAtomComponent::GetStatus()
{
	if (Player == NULL) {
		return EAtomComponentStatus::Error;
	}

	/* AtomExプレーヤのステータスを取得 */
	CriAtomExPlayerStatus PlayerStatus = criAtomExPlayer_GetStatus(Player);

	/* コンポーネントのステータスに変換 */
	EAtomComponentStatus ComponentStatus;
	switch (PlayerStatus) {
		case CRIATOMEXPLAYER_STATUS_STOP:
			ComponentStatus = EAtomComponentStatus::Stop;
			break;

		case CRIATOMEXPLAYER_STATUS_PREP:
			ComponentStatus = EAtomComponentStatus::Prep;
			break;

		case CRIATOMEXPLAYER_STATUS_PLAYING:
			ComponentStatus = EAtomComponentStatus::Playing;
			break;

		case CRIATOMEXPLAYER_STATUS_PLAYEND:
			ComponentStatus = EAtomComponentStatus::PlayEnd;
			break;

		default:
			ComponentStatus = EAtomComponentStatus::Error;
			break;
	}

	return ComponentStatus;
}

void UAtomComponent::Pause(bool bPause)
{
	if (Player == NULL) {
		return;
	}

	if (bPause) {
		criAtomExPlayer_Pause(Player, CRI_ON);
	} else {
		criAtomExPlayer_Resume(Player, CRIATOMEX_RESUME_PAUSED_PLAYBACK);
	}
}

bool UAtomComponent::IsPaused()
{
	if (Player == NULL) {
		return false;
	}

	if (criAtomExPlayer_IsPaused(Player) == CRI_TRUE) {
		return true;
	} else {
		return false;
	}
}

bool UAtomComponent::IsPlaying(void)
{
	/* ステータスの確認 */
	EAtomComponentStatus Status = GetStatus();
	if ((Status == EAtomComponentStatus::Prep) || (Status == EAtomComponentStatus::Playing)) {
		return true;
	} else {
		return false;
	}
}

/* ボリュームの変更 */
void UAtomComponent::SetVolume(float Volume)
{
	/* ボリューム値の保存 */
	DefaultVolume = Volume;

	if (Player == NULL) {
		return;
	}

#if !defined(CRIWARE_UE4_LE)	/* <cri_delete_if_LE> */
#endif	/* </cri_delete_if_LE> */

	/* ボリュームの更新 */
	criAtomExPlayer_SetVolume(Player, Volume);
	criAtomExPlayer_UpdateAll(Player);
}

/* サウンドオブジェクトの設定 */
void UAtomComponent::SetSoundObject(UAtomSoundObject * SoundObject)
{
	if (SoundObject == NULL) {
		return;
	}

	DefaultSoundObject = SoundObject;

	if (Player == NULL) {
		return;
	}

	/* サウンドオブジェクトの設定 */
	criAtomExSoundObject_AddPlayer(SoundObject->GetSoundObjectHandle(), Player);
}

void UAtomComponent::SetPitchMultiplier(float NewPitchMultiplier)
{
	/* ピッチをセント単位に変更 */
	float Cent = 1200.0f * FMath::Log2(NewPitchMultiplier);

	/* ピッチの変更 */
	SetPitch(Cent);
}

/* ピッチの変更 */
void UAtomComponent::SetPitch(float Pitch)
{
	if (Player != NULL) {
		criAtomExPlayer_SetPitch(Player, Pitch);
		criAtomExPlayer_UpdateAll(Player);
	}
}

/* AISACコントロール値の指定 */
void UAtomComponent::SetAisacByName(FString ControlName, float ControlValue)
{
	if (Player != NULL) {
		criAtomExPlayer_SetAisacByName(Player, TCHAR_TO_UTF8(*ControlName), ControlValue);
		criAtomExPlayer_UpdateAll(Player);
	}
}

/* 非推奨関数 */
void UAtomComponent::SetBusSendLevel(int32 BusId, float Level)
{
	if (Player != NULL) {
		criAtomExPlayer_SetBusSendLevel(Player, BusId, Level);
		criAtomExPlayer_UpdateAll(Player);
	}
}

/* バスセンドレベルの設定 */
void UAtomComponent::SetBusSendLevelByName(FString BusName, float Level)
{
	if (Player != NULL) {
		criAtomExPlayer_SetBusSendLevelByName(Player, TCHAR_TO_UTF8(*BusName), Level);
		criAtomExPlayer_UpdateAll(Player);
	}
}

/* 非推奨関数 */
void UAtomComponent::SetBusSendLevelOffset(int32 BusId, float LevelOffset)
{
	if (Player != NULL) {
		criAtomExPlayer_SetBusSendLevelOffset(Player, BusId, LevelOffset);
		criAtomExPlayer_UpdateAll(Player);
	}
}

/* バスセンドレベルの設定（オフセット指定） */
void UAtomComponent::SetBusSendLevelOffsetByName(FString BusName, float LevelOffset)
{
	if (Player != NULL) {
		criAtomExPlayer_SetBusSendLevelOffsetByName(Player, TCHAR_TO_UTF8(*BusName), LevelOffset);
		criAtomExPlayer_UpdateAll(Player);
	}
}

/* ブロックの遷移 */
void UAtomComponent::SetNextBlockIndex(int32 BlockIndex)
{
	DefaultBlockIndex = BlockIndex;
	criAtomExPlayback_SetNextBlockIndex(PlaybackId, BlockIndex);
}

/* セレクタラベルの設定 */
void UAtomComponent::SetSelectorLabel(FString Selector, FString Label)
{
	if (Player != NULL) {
		criAtomExPlayer_SetSelectorLabel(Player, TCHAR_TO_UTF8(*Selector), TCHAR_TO_UTF8(*Label));
		criAtomExPlayer_UpdateAll(Player);
	}
}

#if !defined(CRIWARE_UE4_LE)	/* <cri_delete_if_LE> */
#endif	/* </cri_delete_if_LE> */

float UAtomComponent::GetTime()
{
	CriSint64 TimeMS = criAtomExPlayback_GetTime(PlaybackId);
	if (TimeMS < 0) {
		return -1.0f;
	} else {
		return ((float)TimeMS * 0.001f);
	}
}

float UAtomComponent::GetSequencePosition()
{
	CriSint64 TimeMS = criAtomExPlayback_GetSequencePosition(PlaybackId);
	if (TimeMS < 0) {
		return -1.0f;
	} else {
		return ((float)TimeMS * 0.001f);
	}
}

/* AtomExプレーヤハンドルの取得 */
CriAtomExPlayerHn UAtomComponent::GetAtomExPlayer()
{
	return Player;
}

/* AtomEx3dSourceハンドルの取得 */
CriAtomEx3dSourceHn UAtomComponent::GetAtomEx3dSource()
{
	return Source;
}

void UAtomComponent::PostInitProperties()
{
	Super::PostInitProperties();

	/* デフォルトオブジェクトかどうかチェック */
	if (HasAnyFlags(RF_ClassDefaultObject)) {
		/* デフォルトオブジェクトの場合は何もしない */
		return;
	}

	/* シームレス連結再生用にプレイリストを作成 */
	Playlist = new FAtomPlaylist();

	/* 注意）テンプレートクラス等で処理負荷が上がらないよう、	*/
	/* 　　　PostInitProperties時点ではPlayer等は確保しない。	*/
}

UAtomComponent* UAtomComponent::GetAtomComponentFromID(uint64 AtomComponentID)
{
	check(IsInGameThread());
	return AtomIDToComponentMap.FindRef(AtomComponentID);
}

void UAtomComponent::BeginDestroy()
{
	/* 再生の停止 */
	Stop();

	/* リソースの破棄 */
	ReleaseResource();

	/* シームレス連結再生用のプレイリストを破棄 */
	if (Playlist != nullptr) {
		delete Playlist;
		Playlist = nullptr;
	}

#if WITH_EDITOR
	/* デリゲートの登録解除 */
	if (OnWorldCleanedUpHandle.IsValid() != false) {
		FWorldDelegates::OnWorldCleanup.Remove(OnWorldCleanedUpHandle);
		OnWorldCleanedUpHandle.Reset();
	}
#endif

	/* ルート化されている場合は解除 */
	/* 注意）AtomRootedComponentArrayのイテレーション中に登録要素を			*/
	/* 　　　破棄できないため、この処理はReleaseResource内に移動できない。	*/
	if (bPersistAcrossLevelTransition) {
		AtomRootedComponentArray.Remove(this);
	}

	/* プロファイラで列挙されないようComponentをMapから削除 */
	if (!HasAnyFlags(RF_ClassDefaultObject)) {
		AtomIDToComponentMap.Remove(AtomComponentID);
	}

	Super::BeginDestroy();
}

void UAtomComponent::DestroyComponent(bool bPromoteChildren/*= false*/)
{
	/* 再生の停止 */
	Stop();

	/* リソースの破棄 */
	ReleaseResource();

	/* ルート化の解除 */
	/* 注意）この処理はSuper::DestroyComponent前に実行しておく必要がある。	*/
	/* 　　　（BeginDestroy内で行うとAssertに引っかかる。）					*/
	if (bPersistAcrossLevelTransition) {
		RemoveFromRoot();
	}

	Super::DestroyComponent(bPromoteChildren);
}

#if WITH_EDITOR
void UAtomComponent::OnAttachmentChanged()
{
	Super::OnAttachmentChanged();

	if (GetAttachParent() == NULL) {
		/* 音源表示用コンポーネントの破棄 */
		if (MaxDistanceSphereComponent) {
			MaxDistanceSphereComponent->DestroyComponent();
		}
		if (MinDistanceSphereComponent) {
			MinDistanceSphereComponent->DestroyComponent();
		}
	}
}

void UAtomComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	/* 再生をリスタート */
	Play();

#if WITH_EDITORONLY_DATA
	/* Auto Activateプロパティの変化に合わせて、スピーカーアイコンを変更 */
	UpdateSpriteTexture();
#endif

	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif

void UAtomComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	OnTickComponent();
}

void UAtomComponent::OnTickComponent()
{
	/* 再生が開始されたかどうかチェック */
	if ((Player == nullptr) || (bIsActive == false)) {
		/* 再生前は何もしない */
		return;
	}

#if !defined(CRIWARE_UE4_LE)	/* <cri_delete_if_LE> */
#endif	/* </cri_delete_if_LE> */

	/* プレーヤのステータスをチェック */
	CriAtomExPlayerStatus Status = criAtomExPlayer_GetStatus(Player);
	if ((Status == CRIATOMEXPLAYER_STATUS_PREP) || (Status == CRIATOMEXPLAYER_STATUS_PLAYING)) {
#if WITH_EDITOR
		/* Play In Editorモードでは、再生中に音源位置と減衰距離を表示する */
		if (GIsPlayInEditorWorld) {
			/* コンソール変数 "cri.ShowAtomSoundActor" が 0 より大きいときのみ表示する。 */
			static const auto bShowSoundLoation = IConsoleManager::Get().FindTConsoleVariableDataInt(TEXT("cri.ShowSoundLocation"));
			if (bShowSoundLoation->GetValueOnGameThread() > 0) {
				/* Play In Editorモードで表示するには、ActorとComponentのbHiddenGameプロパティを、両方とも false にする必要がある。 */
				if (GetOwner()) {
					GetOwner()->SetActorHiddenInGame(false);
				}
				SetHiddenInGame(false, true);

				/* Attenuationパラメータ設定時の描画処理 */
				DrawDebugShape();
			}
		}
#endif

		/* 再生済みのデータを破棄 */
		if ((Playlist != nullptr) && (Status == CRIATOMEXPLAYER_STATUS_PLAYING)) {
			/* 排他制御区間の開始 */
			Playlist->Lock();

			/* 備考）解放待ち、再生中、再生待ちの3パケットは保持しておく。 */
			int32 Pos = Playlist->Tell();
			if (Pos > 3) {
				Playlist->RemoveAt(0);
			}

			/* 排他制御区間の終了 */
			Playlist->Unlock();
		}

		return;
	}

	/* 再生の完了を通知 */
	if (OnAudioFinished.IsBound()) {
		OnAudioFinished.Broadcast();
	}
	if (OnAudioFinishedNative.IsBound()) {
		OnAudioFinishedNative.Broadcast(this);
	}

	if (bAutoDestroy) {
		/* 再生の停止 */
		Stop();

		/* リソースの破棄 */
		/* 備考）GCまでの間ハンドルを保持し続けると、サーバ処理内でリストを     */
		/* 　　　辿る処理が無駄に発生し続けるので、ここでハンドルを破棄する。   */
		ReleaseResource();

		/* 再生終了時はコンポーネントを破棄 */
		DestroyComponent();

		/* 音源表示用コンポーネントの破棄 */
#if WITH_EDITOR
		if (MaxDistanceSphereComponent) {
			MaxDistanceSphereComponent->DestroyComponent();
		}
		if (MinDistanceSphereComponent) {
			MinDistanceSphereComponent->DestroyComponent();
		}
#endif
	} else {
#if WITH_EDITOR
		/* 再生終了したので、音源表示を隠す */
		if (GIsPlayInEditorWorld) {
			SetHiddenInGame(true, true);
		}
#endif
	}
}

void UAtomComponent::Activate(bool bReset)
{
	if (bReset || ShouldActivate() == true) {
		Play();
	}
}

void UAtomComponent::Deactivate()
{
	if (ShouldActivate() == false) {
		Stop();
	}
}

#if !defined(CRIWARE_UE4_LE)	/* <cri_delete_if_LE> */
#endif	/* </cri_delete_if_LE> */

#if !defined(CRIWARE_UE4_LE)	/* <cri_delete_if_LE> */
#endif	/* </cri_delete_if_LE> */


/* スピーカーアイコン表示関連 */
#if WITH_EDITORONLY_DATA
/* Auto Activeならばチェックボックス付きのテクスチャを使う */
void UAtomComponent::UpdateSpriteTexture()
{
	if (SpriteComponent) {
		if (bAutoActivate) {
			SpriteComponent->SetSprite(LoadObject<UTexture2D>(NULL, TEXT("/Engine/EditorResources/AudioIcons/S_AudioComponent_AutoActivate.S_AudioComponent_AutoActivate")));
		} else {
			SpriteComponent->SetSprite(LoadObject<UTexture2D>(NULL, TEXT("/Engine/EditorResources/AudioIcons/S_AudioComponent.S_AudioComponent")));
		}
	}
}

void UAtomComponent::OnRegister()
{
	Super::OnRegister();

	/* エディターのパースペクティブビューに、スピーカーのアイコンを表示
	* インゲームでも、デバッグ表示向けにSpriteComponentを初期化しておく */
	/* SceneComponent.cpp USceneComponent::OnRegisterを参考 */
	if (bVisualizeComponent && SpriteComponent == nullptr && GetOwner()) {
		SpriteComponent = NewObject<UBillboardComponent>(GetOwner(), NAME_None, RF_Transactional | RF_TextExportTransient);

		SpriteComponent->Sprite = LoadObject<UTexture2D>(nullptr, TEXT("/Engine/EditorResources/EmptyActor.EmptyActor"));
		SpriteComponent->RelativeScale3D = FVector(0.5f, 0.5f, 0.5f);
		SpriteComponent->Mobility = EComponentMobility::Movable;
		SpriteComponent->AlwaysLoadOnClient = false;
		SpriteComponent->AlwaysLoadOnServer = false;
		SpriteComponent->SpriteInfo.Category = TEXT("Misc");
		SpriteComponent->SpriteInfo.DisplayName = NSLOCTEXT("SpriteCategory", "Misc", "Misc");
		SpriteComponent->CreationMethod = CreationMethod;
		SpriteComponent->bIsScreenSizeScaled = true;
		SpriteComponent->bUseInEditorScaling = true;

		SpriteComponent->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);
		SpriteComponent->RegisterComponent();
	}
	UpdateSpriteTexture();

#if !defined(CRIWARE_UE4_LE)	/* <cri_delete_if_LE> */
#endif	/* </cri_delete_if_LE> */
}
#endif /* WITH_EDITORONLY_DATA */

void UAtomComponent::OnUnregister()
{
	Super::OnUnregister();

	AActor* Owner = GetOwner();
	if (!Owner || bStopWhenOwnerDestroyed) {
		Stop();
	}
}

void UAtomComponent::OnUpdateTransform(EUpdateTransformFlags UpdateTransformFlags, ETeleportType Teleport)
{
	Super::OnUpdateTransform(UpdateTransformFlags, Teleport);

	/* 音源の位置を更新 */
	UpdateTransform();
}

void UAtomComponent::UpdateTransform(void)
{
	if (Source != NULL) {
		/* ソースの座標を更新 */
		const FVector SourcePosition = GetComponentLocation();
		CriAtomExVector pos;
		pos.x = SourcePosition.X * DistanceFactor;
		pos.y = SourcePosition.Y * DistanceFactor;
		pos.z = SourcePosition.Z * DistanceFactor;
		criAtomEx3dSource_SetPosition(Source, &pos);

		/* ソースの向きを更新 */
		const FRotator SourceRotation = GetComponentRotation();
		FVector FrontVector = SourceRotation.Vector();
		CriAtomExVector front;
		front.x = FrontVector.X;
		front.y = FrontVector.Y;
		front.z = FrontVector.Z;
		criAtomEx3dSource_SetConeOrientation(Source, &front);

		/* 更新の適用 */
		criAtomEx3dSource_Update(Source);
	}
};

void UAtomComponent::AllocateResource()
{
	/* デフォルトオブジェクトかどうかチェック */
	if (HasAnyFlags(RF_ClassDefaultObject)) {
		/* デフォルトオブジェクトの場合は何もしない */
		return;
	}

	/* プレーヤハンドルが確保済みの場合は何もしない */
	if (Player != NULL) {
		return;
	}

	/* リスナの取得 */
	CriAtomEx3dListenerHn Listener = UCriWareInitializer::GetListener();
	if (Listener == NULL) {
		/* リスナが未作成の場合は何もしない */
		return;
	}

	/* ソースの作成 */
	Source = criAtomEx3dSource_Create(NULL, SourceWork, sizeof(SourceWork));
	if (Source == NULL) {
		/* ソースが確保できない場合は何もしない */
		return;
	}

	/* プレーヤの作成 */
	Player = criAtomExPlayer_Create(NULL, PlayerWork, sizeof(PlayerWork));
	if (Player == NULL) {
		/* プレーヤが確保できない場合はソースも破棄 */
		criAtomEx3dSource_Destroy(Source);
		Source = NULL;
		return;
	}

	/* データ要求コールバックを設定 */
	criAtomExPlayer_SetDataRequestCallback(Player, UAtomComponent::OnDataRequest, Playlist);

	/* 距離係数の取得 */
	DistanceFactor = UCriWareInitializer::GetDistanceFactor();

	/* ソース、リスナをプレーヤに関連付け */
	criAtomExPlayer_Set3dListenerHn(Player, Listener);
	criAtomExPlayer_Set3dSourceHn(Player, Source);

	/* 7.1ch環境向けにパンスピーカータイプを設定 */
	criAtomExPlayer_SetPanSpeakerType(Player, CRIATOMEX_PAN_SPEAKER_TYPE_6CH);

	/* サウンドオブジェクトを実際に設定 */
	SetSoundObject(DefaultSoundObject);
}

void UAtomComponent::ReleaseResource()
{
	/* デフォルトオブジェクトかどうかチェック */
	if (HasAnyFlags(RF_ClassDefaultObject)) {
		/* デフォルトオブジェクトの場合は何もしない */
		return;
	}

	/* キューシートのリリース */
	if (CueSheet != NULL) {
		CueSheet->Release();
		CueSheet = NULL;
	}

	/* サウンドの参照を破棄 */
	Sound = nullptr;

	/* ソースとプレーヤを破棄 */
	/* 注意）GC処理の順序によっては、ライブラリ終了処理後にここに来る可能性がある。 */
	if (criAtomEx_IsInitialized() != CRI_FALSE) {
		if (Player != NULL) {
			criAtomExPlayer_Destroy(Player);
		}
		if (Source != NULL) {
			criAtomEx3dSource_Destroy(Source);
		}
	}

	/* パラメータのクリア */
	Source = NULL;
	Player = NULL;
}

#if WITH_EDITOR
void UAtomComponent::DrawDebugShape()
{
	#if !defined(CRIWARE_UE4_LE)	/* <cri_delete_if_LE> */
#endif	/* </cri_delete_if_LE> */
}

void UAtomComponent::OnWorldCleanedUp(UWorld* World, bool bSessionEnded, bool bCleanupResources)
{
	/* 自身が所属するワールドかどうかチェック */
	UWorld* MyWorld = GetWorld();
	if (MyWorld != World) {
		return;
	}

	/* セッション終了時は音声を停止 */
	if (bSessionEnded || bCleanupResources) {
		Stop();
	}

	/* リソース破棄要求時はコンポーネントの破棄を要求 */
	if (bCleanupResources) {
		DestroyComponent();
	}
}
#endif

/***************************************************************************
 *      関数定義
 *      Function Definition
 ***************************************************************************/
#undef LOCTEXT_NAMESPACE

/* --- end of file --- */
