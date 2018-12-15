/****************************************************************************
 *
 * CRI Middleware SDK
 *
 * Copyright (c) 2013-2017 CRI Middleware Co., Ltd.
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

/* Unreal Engine 4関連ヘッダ */
#include "AudioThread.h"
#include "UObject/Package.h"
#include "Misc/PackageName.h"
#include "Engine/LocalPlayer.h"
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
uint64 UAtomComponent::AtomComponentIDCounter = 0;

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
	bStopWhenOwnerDestroyed = true;
	DefaultVolume = 1.0f;
	bEnableMultipleSoundPlayback = false;
	DefaultBlockIndex = 0;
	Player = NULL;
	Source = NULL;
	DistanceFactor = 1.0f;
	CueSheet = NULL;
	PlaybackId = 0xFFFFFFFF;

	/* 音源位置更新のため、OnUpdateTransformを呼び出してもらうよう指定する。 */
	bWantsOnUpdateTransform = true;

	/* 不要な機能を無効化 */
	bNeverNeedsRenderUpdate = true;

#if WITH_EDITORONLY_DATA
	/* 音源のスピーカアイコン表示を有効にする */
	bVisualizeComponent = true;
#endif

	AtomComponentID = ++AtomComponentIDCounter;

#if WITH_EDITOR
	/* デリゲート登録のリセット */
	OnWorldCleanedUpHandle.Reset();
#endif
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
}

void UAtomComponent::Play(float StartTime)
{
	/* 既に再生中の場合は一旦停止 */
	if (bEnableMultipleSoundPlayback == false) {
		Stop();
	}

	/* 発音が可能かどうかチェック */
	if (Player == nullptr) {
		/* プレーヤが存在しない場合は再生不可 */
		return;
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
		if (CueSheet == nullptr) {
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

	/* キュー名の指定 */
	CriAtomExAcbHn AcbHn = ((CueSheet != nullptr) ? CueSheet->GetAcbHn() : nullptr);
	criAtomExPlayer_SetCueName(Player, AcbHn, TCHAR_TO_UTF8(*CueName));

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

#if WITH_EDITOR
	/* エディター上（非プレビュー中）のみ鳴らす音はパン3Dで再生 */
	/* 注意）シーケンサーで再生する音は常にUIサウンドとして再生されるため、	*/
	/* 　　　UIサウンドであっても暗黙でパン3Dを適用してはいけない。			*/
	if ((bIsGameWorld == false) && (bIsUISound == true)) {
		criAtomExPlayer_SetPanType(Player, CRIATOMEX_PAN_TYPE_PAN3D);
	}
#endif

	/* サウンドオブジェクトの設定 */
	if (DefaultSoundObject != NULL) {
		criAtomExSoundObject_AddPlayer(DefaultSoundObject->GetSoundObjectHandle(), Player);
	}

	/* 距離減衰の上書き判定 */
	if ((bOverrideAttenuation != false) || (AttenuationSettings != NULL)) {
		/* 距離減衰上書き時はPan3Dとして再生 */
		criAtomExPlayer_SetPanType(Player, CRIATOMEX_PAN_TYPE_PAN3D);

		/* 減衰ボリュームの計算 */
		float Volume = DefaultVolume;
		ApplyAttenuation(Volume, Filter_Frequency);

		/* ボリュームの更新 */
		criAtomExPlayer_SetVolume(Player, Volume);

		/* LPFの適用 */
		const auto* Settings = GetAttenuationSettingsToApply();
		criAtomExPlayer_SetBandpassFilterParameters(Player, 0.f, Filter_Frequency / MAX_FILTER_FREQUENCY);
	}

	/* 再生の開始 */
	PlaybackId = criAtomExPlayer_Start(Player);

	/* 再生の開始を通知 */
	bIsActive = true;

#if WITH_EDITOR
	/* Play In Editorモードで減衰距離を表示するため、キュー情報から取得 */
	if (GIsPlayInEditorWorld && MaxDistanceSphereComponent && MinDistanceSphereComponent && Sound != NULL) {
		MaxDistanceSphereComponent->SetSphereRadius(Sound->GetMaxAttenuationDistance() / DistanceFactor, false);
		MinDistanceSphereComponent->SetSphereRadius(Sound->GetMinAttenuationDistance() / DistanceFactor, false);
	}
#endif
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

	/* 変更の適用 */
	if (Player != NULL) {
		/* 距離減衰の上書き判定 */
		if ((bOverrideAttenuation != false) || (AttenuationSettings != NULL)) {
			/* 減衰ボリュームの計算 */
			ApplyAttenuation(Volume, Filter_Frequency);

			/* LPFの適用 */
			const auto* Settings = GetAttenuationSettingsToApply();
			criAtomExPlayer_SetBandpassFilterParameters(Player, 0.f, Filter_Frequency / MAX_FILTER_FREQUENCY);
		}

		/* ボリュームの更新 */
		criAtomExPlayer_SetVolume(Player, Volume);

		criAtomExPlayer_UpdateAll(Player);
	}
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

	/* リソースの確保 */
	AllocateResource();
}

void UAtomComponent::BeginDestroy()
{
	/* 再生の停止 */
	Stop();

	/* リソースの破棄 */
	ReleaseResource();

#if WITH_EDITOR
	/* デリゲートの登録解除 */
	if (OnWorldCleanedUpHandle.IsValid() != false) {
		FWorldDelegates::OnWorldCleanup.Remove(OnWorldCleanedUpHandle);
		OnWorldCleanedUpHandle.Reset();
	}
#endif

	Super::BeginDestroy();
}

void UAtomComponent::PostLoad()
{
	Super::PostLoad();
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

	/* 再生が開始されたかどうかチェック */
	if ((Player == nullptr) || (bIsActive == false)) {
		/* 再生前は何もしない */
		return;
	}

	/* 距離減衰の上書き判定 */
	if (((bOverrideAttenuation == true) || (AttenuationSettings != NULL)) && (bPersistAcrossLevelTransition == false)) {
		/* 減衰ボリュームの計算 */
		float Volume = DefaultVolume;
		ApplyAttenuation(Volume, Filter_Frequency);

		/* ボリュームの更新 */
		criAtomExPlayer_SetVolume(Player, Volume);

		/* LPFの適用 */
		const auto* Settings = GetAttenuationSettingsToApply();
		criAtomExPlayer_SetBandpassFilterParameters(Player, 0.f, Filter_Frequency / MAX_FILTER_FREQUENCY);

		criAtomExPlayer_UpdateAll(Player);
	}

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
		/* 再生中は何もしない */
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

const FSoundAttenuationSettings* UAtomComponent::GetAttenuationSettingsToApply() const
{
	/* AttenuationSettingsを使用するかどうかチェック */
	if (bOverrideAttenuation != false) {
		/* AttenuationSettings上書き時 */
		return &AttenuationOverrides;
	} else if (AttenuationSettings != NULL) {
		/* AttenuationSettingsアセット使用時 */
		return &AttenuationSettings->Attenuation;
	}

	return NULL;
}

void UAtomComponent::ApplyAttenuation(float& Volume, float& FilterFrequency)
{
	AtomActiveSound.SetAtomComponent(this->GetOwner(), this->GetAtomComponentID(), this->GetFName());
	AtomActiveSound.SetWorld(GetWorld());

	/* AttenuationSettingsの取得 */
	const auto* Settings = GetAttenuationSettingsToApply();
	if (Settings == NULL) {
		return;
	}

	/* ソースの向きと座標を取得 */
	const FRotator SourceRotation = GetComponentRotation();
	const FVector SourcePosition = GetComponentLocation();

	/* リスニングポイントの座標を取得 */
	FVector ListenerLocation = UCriWareInitializer::GetListeningPoint();

	/* ボリュームの初期値を計算 */
	FTransform SoundTransform = FTransform(SourceRotation, SourcePosition);
	Volume = DefaultVolume;

	/* 距離減衰の計算 */
	/* 備考）UE4.10相当の処理を移植。 */
	if (Settings->bAttenuate) {
		switch (Settings->AttenuationShape) {
			case EAttenuationShape::Sphere:
			{
				const float Distance = FMath::Max(
					FVector::Dist(SoundTransform.GetTranslation(), ListenerLocation)
					- Settings->AttenuationShapeExtents.X, 0.f);
				Volume *= Settings->AttenuationEval(Distance, Settings->FalloffDistance, 1.0f);
				break;
			}

			case EAttenuationShape::Box:
				Volume *= Settings->AttenuationEvalBox(SoundTransform, ListenerLocation, 1.0f);
				break;

			case EAttenuationShape::Capsule:
				Volume *= Settings->AttenuationEvalCapsule(SoundTransform, ListenerLocation, 1.0f);
				break;

			case EAttenuationShape::Cone:
				Volume *= Settings->AttenuationEvalCone(SoundTransform, ListenerLocation, 1.0f);
				break;

			default:
				check(false);
		}
	}
	// Only do occlusion traces if the sound is audible
	if (Settings->bEnableOcclusion && Volume > 0.0f /*&& !AudioDevice->IsAudioDeviceMuted()*/)
	{
		AtomActiveSound.CheckOcclusion(UCriWareInitializer::GetListenerLocation(), GetComponentLocation(), Settings);
		FilterFrequency = AtomActiveSound.CurrentOcclusionFilterFrequency.GetValue();
		// Apply the volume attenuation due to occlusion (using the interpolating dynamic parameter)

		Volume *= AtomActiveSound.CurrentOcclusionVolumeAttenuation.GetValue();
	}
}

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

	/* Play In Editorモードでは、再生中に最大、最小減衰距離を表示する。 */
	if (GIsPlayInEditorWorld && (MaxDistanceSphereComponent == NULL) && GetOwner()
		&& (bOverrideAttenuation == false) && (AttenuationSettings == NULL)) {
		if (MaxDistanceSphereComponent == NULL) {
			MaxDistanceSphereComponent = NewObject<UDrawSphereComponent>(GetOwner(), NAME_None, RF_Transactional | RF_TextExportTransient);

			MaxDistanceSphereComponent->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);
			MaxDistanceSphereComponent->AlwaysLoadOnClient = false;
			MaxDistanceSphereComponent->AlwaysLoadOnServer = false;
			MaxDistanceSphereComponent->CreationMethod = CreationMethod;

			/* 最大減衰距離をキューから情報取得して、描画コンポーネントに設定 */
			if (Sound) {
				auto MaxAttenuationDistance = Sound->GetMaxAttenuationDistance();
				auto SphereRadius = MaxAttenuationDistance / UCriWareInitializer::GetDistanceFactor();
				MaxDistanceSphereComponent->InitSphereRadius(SphereRadius);
			}

			MaxDistanceSphereComponent->RegisterComponent();
		}
		if (MinDistanceSphereComponent == NULL) {
			MinDistanceSphereComponent = NewObject<UDrawSphereComponent>(GetOwner(), NAME_None, RF_Transactional | RF_TextExportTransient);

			MinDistanceSphereComponent->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);
			MinDistanceSphereComponent->AlwaysLoadOnClient = false;
			MinDistanceSphereComponent->AlwaysLoadOnServer = false;
			MinDistanceSphereComponent->CreationMethod = CreationMethod;

			/* 最小減衰距離をキューから情報取得して、描画コンポーネントに設定 */
			if (Sound) {
				auto MinAttenuationDistance = Sound->GetMinAttenuationDistance();
				auto SphereRadius = MinAttenuationDistance / UCriWareInitializer::GetDistanceFactor();
				MinDistanceSphereComponent->InitSphereRadius(SphereRadius);
			}

			MinDistanceSphereComponent->RegisterComponent();
		}
	}
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
		return;
	}

	/* 距離係数の取得 */
	DistanceFactor = UCriWareInitializer::GetDistanceFactor();

	/* ソース、リスナをプレーヤに関連付け */
	criAtomExPlayer_Set3dListenerHn(Player, Listener);
	criAtomExPlayer_Set3dSourceHn(Player, Source);

	/* 7.1ch環境向けにパンスピーカータイプを設定 */
	criAtomExPlayer_SetPanSpeakerType(Player, CRIATOMEX_PAN_SPEAKER_TYPE_6CH);
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
	/* AttenuationSettingsの取得 */
	const auto* Attenuation = GetAttenuationSettingsToApply();
	if (Attenuation == NULL) {
		return;
	}

	/* 減衰範囲の描画に必要なUWorldを取得 */
	UWorld* World = (GEditor ? (GEditor->PlayWorld) : nullptr);
	ULocalPlayer* LocalPlayer = GEngine->GetDebugLocalPlayer();
	if (LocalPlayer) {
		UWorld* PlayerWorld = LocalPlayer->GetWorld();
		if (!World) {
			World = PlayerWorld;
		}
	}
	if (!World) {
		/* 取得失敗時は何もしない */
		return;
	}

	/* 距離減衰描画に必要なパラメータを取得 */
	const FRotator SourceRotation = GetComponentRotation();
	const FVector SourcePosition = GetComponentLocation();
	const FTransform SourceTransform = FTransform(SourceRotation, SourcePosition);

	/* 距離減衰形状の描画 */
	/* 備考）UnrealEngine.cppのUEngine::RenderStatSoundsを参考に作成。 */
	switch (Attenuation->AttenuationShape) {
		case EAttenuationShape::Sphere:
			if (Attenuation->FalloffDistance > 0.0f) {
				DrawDebugSphere(World, SourceTransform.GetTranslation(), Attenuation->AttenuationShapeExtents.X + Attenuation->FalloffDistance, 10, FColor(255, 0, 0));
				DrawDebugSphere(World, SourceTransform.GetTranslation(), Attenuation->AttenuationShapeExtents.X, 10, FColor(155, 0, 0));
			} else {
				DrawDebugSphere(World, SourceTransform.GetTranslation(), Attenuation->AttenuationShapeExtents.X, 10, FColor(255, 0, 0));
			}
			break;

		case EAttenuationShape::Box:
			if (Attenuation->FalloffDistance > 0.0f) {
				DrawDebugBox(World, SourceTransform.GetTranslation(), Attenuation->AttenuationShapeExtents + FVector(Attenuation->FalloffDistance), SourceTransform.GetRotation(), FColor(255, 0, 0));
				DrawDebugBox(World, SourceTransform.GetTranslation(), Attenuation->AttenuationShapeExtents, SourceTransform.GetRotation(), FColor(155, 0, 0));
			} else {
				DrawDebugBox(World, SourceTransform.GetTranslation(), Attenuation->AttenuationShapeExtents, SourceTransform.GetRotation(), FColor(255, 0, 0));
			}
			break;

		case EAttenuationShape::Capsule:
			if (Attenuation->FalloffDistance > 0.0f) {
				DrawDebugCapsule(World, SourceTransform.GetTranslation(), Attenuation->AttenuationShapeExtents.X + Attenuation->FalloffDistance, Attenuation->AttenuationShapeExtents.Y + Attenuation->FalloffDistance, SourceTransform.GetRotation(), FColor(255, 0, 0));
				DrawDebugCapsule(World, SourceTransform.GetTranslation(), Attenuation->AttenuationShapeExtents.X, Attenuation->AttenuationShapeExtents.Y, SourceTransform.GetRotation(), FColor(155, 0, 0));
			} else {
				DrawDebugCapsule(World, SourceTransform.GetTranslation(), Attenuation->AttenuationShapeExtents.X, Attenuation->AttenuationShapeExtents.Y, SourceTransform.GetRotation(), FColor(255, 0, 0));
			}
			break;

		case EAttenuationShape::Cone:
		{
			const FVector Origin = SourceTransform.GetTranslation() - (SourceTransform.GetUnitAxis(EAxis::X) * Attenuation->ConeOffset);

			if (Attenuation->FalloffDistance > 0.0f || Attenuation->AttenuationShapeExtents.Z > 0.0f) {
				const float OuterAngle = FMath::DegreesToRadians(Attenuation->AttenuationShapeExtents.Y + Attenuation->AttenuationShapeExtents.Z);
				const float InnerAngle = FMath::DegreesToRadians(Attenuation->AttenuationShapeExtents.Y);
				DrawDebugCone(World, Origin, SourceTransform.GetUnitAxis(EAxis::X), Attenuation->AttenuationShapeExtents.X + Attenuation->FalloffDistance + Attenuation->ConeOffset, OuterAngle, OuterAngle, 10, FColor(255, 0, 0));
				DrawDebugCone(World, Origin, SourceTransform.GetUnitAxis(EAxis::X), Attenuation->AttenuationShapeExtents.X + Attenuation->ConeOffset, InnerAngle, InnerAngle, 10, FColor(155, 0, 0));
			} else {
				const float Angle = FMath::DegreesToRadians(Attenuation->AttenuationShapeExtents.Y);
				DrawDebugCone(World, Origin, SourceTransform.GetUnitAxis(EAxis::X), Attenuation->AttenuationShapeExtents.X + Attenuation->ConeOffset, Angle, Angle, 10, FColor(255, 0, 0));
			}
		}
		break;

		default:
			check(false);
			break;
	}
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
