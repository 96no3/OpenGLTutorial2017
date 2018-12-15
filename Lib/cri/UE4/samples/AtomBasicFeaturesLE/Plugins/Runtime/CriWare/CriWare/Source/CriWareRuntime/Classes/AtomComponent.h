/****************************************************************************
 *
 * CRI Middleware SDK
 *
 * Copyright (c) 2013-2017 CRI Middleware Co., Ltd.
 *
 * Library  : CRIWARE plugin for Unreal Engine 4
 * Module   : AudioComponent for Atom
 * File     : AtomComponent.h
 *
 ****************************************************************************/

/* 多重定義防止 */
#pragma once

/***************************************************************************
 *      インクルードファイル
 *      Include files
 ***************************************************************************/
/* Unreal Engine 4関連ヘッダ */
#include "Components/SceneComponent.h"
#include "Sound/SoundAttenuation.h"
#include "GCObject.h"
#if WITH_EDITORONLY_DATA
#include "Runtime/Engine/Classes/Components/BillboardComponent.h"
#include "Runtime/Engine/Classes/Components/DrawSphereComponent.h"
#endif

/* CRIWAREプラグインヘッダ */
#include "SoundAtomCueSheet.h"
#include "SoundAtomCue.h"
#include "AtomSoundObject.h"
#include "AtomActiveSound.h"

/* モジュールヘッダ */
#include "AtomComponent.generated.h"

/***************************************************************************
 *      定数マクロ
 *      Macro Constants
 ***************************************************************************/

/***************************************************************************
 *      処理マクロ
 *      Macro Functions
 ***************************************************************************/

/***************************************************************************
 *      データ型宣言
 *      Data Type Declarations
 ***************************************************************************/
UENUM(BlueprintType)
enum class EAtomComponentStatus : uint8
{
	Stop,				/** 停止中 */
	Prep,				/** 再生準備中 */
	Playing,			/** 再生中 */
	PlayEnd,			/** 再生終了 */
	Error				/** エラー */
};

struct FListener;
struct FAtomActiveSound;

/***************************************************************************
 *      変数宣言
 *      Prototype Variables
 ***************************************************************************/

/***************************************************************************
 *      クラス宣言
 *      Prototype Classes
 ***************************************************************************/

/* AISACコントロールパラメータ */
USTRUCT(BlueprintType, meta = (ToolTip = "AtomAisacControlParam struct."))
struct FAtomAisacControlParam
{
	GENERATED_USTRUCT_BODY()

	FAtomAisacControlParam();

	/* AISACコントロール名 */
	UPROPERTY(EditAnywhere, Category = "Aisac", meta = (ToolTip = "Name."))
	FString Name;

	/* AISACコントロール値 */
	UPROPERTY(EditAnywhere, Category = "Aisac", meta = (ToolTip = "Value.", ClampMin = "0.0", ClampMax = "1.0"))
	float Value;
};

/* セレクタパラメータ */
USTRUCT(BlueprintType, meta = (ToolTip = "AtomAisacControlParam struct."))
struct FAtomSelectorParam
{
	GENERATED_USTRUCT_BODY()

	FAtomSelectorParam();

	/* セレクタ名 */
	UPROPERTY(EditAnywhere, Category = "Selector", meta = (ToolTip = "Name."))
	FString Selector;

	/* ラベル名 */
	UPROPERTY(EditAnywhere, Category = "Selector", meta = (ToolTip = "Name."))
	FString Label;
};

UCLASS(ClassGroup = (Audio, Common),
	hidecategories = (Object, ActorComponent, Physics, Rendering, Mobility, LOD),
	ShowCategories = Trigger,
	meta = (BlueprintSpawnableComponent, ToolTip = "AtomComponent class.")
)
class CRIWARERUNTIME_API UAtomComponent : public USceneComponent
{
	GENERATED_UCLASS_BODY()

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAudioFinished);
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnAudioFinishedNative, class UAtomComponent*);

	/* 音声データ */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sound", meta = (ToolTip = "Sound data."))
	class USoundAtomCue* Sound;

	/* 再生完了と同時にコンポーネントを破棄するかどうか */
	UPROPERTY(meta = (ToolTip = "Auto destroy."))
	uint32 bAutoDestroy : 1;

	/* パーシスタントレベルをまたいで音声を再生するかどうか */
	UPROPERTY(meta = (ToolTip = "Is PersistAcrossLevelTransitionSound?."))
	bool bPersistAcrossLevelTransition;

	/* オーナー破棄時に音声を停止するかどうか */
	UPROPERTY(meta = (ToolTip = "Stop when owner destroyed."))
	uint32 bStopWhenOwnerDestroyed : 1;

	/* ゲーム中でなくても、PersonaなどのUI向けに再生するかどうか */
	UPROPERTY(meta = (ToolTip = "Is UI sound?"))
	uint32 bIsUISound : 1;

	/* デフォルトのボリューム値 */
	UPROPERTY(EditAnywhere, Category = "Sound", meta = (DisplayName = "Volume Multiplier", ToolTip = "Volume multiplier.", ClampMin = "0.0", ClampMax = "1.0"))
	float DefaultVolume;

	/* １つのコンポーネントで複数の音を同時に再生可能かどうか */
	UPROPERTY(EditAnywhere, Category = "Atom", meta = (ToolTip = "Enable multiple sound playback."))
	bool bEnableMultipleSoundPlayback;

	/* SoundObject */
	UPROPERTY(EditAnywhere, Category = "Atom", meta = (DisplayName = "Sound Object", ToolTip = "Sound object."))
	class UAtomSoundObject* DefaultSoundObject;

	/* デフォルトのブロックインデックス */
	UPROPERTY(EditAnywhere, Category = "Atom", meta = (DisplayName = "Block Index", ToolTip = "Initial block index.", ClampMin = "0"))
	int32 DefaultBlockIndex;

	/* デフォルトのAISAC設定 */
	UPROPERTY(EditAnywhere, Category = "Atom", meta = (DisplayName = "AISAC Control", ToolTip = "Initial AISAC control."))
	TArray<struct FAtomAisacControlParam> DefaultAisacControl;

	/* デフォルトのセレクタラベル設定 */
	UPROPERTY(EditAnywhere, Category = "Atom", meta = (DisplayName = "Selector Label", ToolTip = "Initial selector label."))
	TArray<struct FAtomSelectorParam> DefaultSelectorLabel;

	/** called when we finish playing audio, either because it played to completion or because a Stop() call turned it off early */
	UPROPERTY(BlueprintAssignable)
	FOnAudioFinished OnAudioFinished;

	/** shadow delegate for non UObject subscribers */
	FOnAudioFinishedNative OnAudioFinishedNative;

	/* 音声データのセット */
	UFUNCTION(BlueprintCallable, Category = "AtomSound", meta = (ToolTip = "Set sound."))
	void SetSound(USoundAtomCue* NewSound);

	/* 再生開始 */
	UFUNCTION(BlueprintCallable, Category = "AtomSound", meta = (ToolTip = "Start playing."))
	void Play(float StartTime = 0.f);

	/* 再生停止 */
	UFUNCTION(BlueprintCallable, Category = "AtomSound", meta = (ToolTip = "Stop playing."))
	void Stop();

	/** 再生状態の取得 */
	UFUNCTION(BlueprintCallable, Category = "AtomSound", meta = (ToolTip = "Get playback status."))
	EAtomComponentStatus GetStatus();

	/* 一時停止 */
	UFUNCTION(BlueprintCallable, Category = "AtomSound", meta = (ToolTip = "Pause playing."))
	void Pause(bool bPause);

	/* 一時停止中かどうか */
	UFUNCTION(BlueprintCallable, Category = "AtomSound", meta = (ToolTip = "Is paused?"))
	bool IsPaused();

	/* 再生中かどうか */
	UFUNCTION(BlueprintCallable, Category = "AtomSound", meta = (ToolTip = "Is playing?"))
	bool IsPlaying();

	/* ボリュームの変更 */
	UFUNCTION(BlueprintCallable, Category = "AtomSound", meta = (ToolTip = "Set volume."))
	void SetVolume(float Volume);

	/** Set a new pitch multiplier */
	UFUNCTION(BlueprintCallable, Category = "AtomSound", meta = (ToolTip = "Set pitch."))
	void SetPitchMultiplier(float NewPitchMultiplier);

	/* ピッチの変更（セント単位） */
	UFUNCTION(BlueprintCallable, Category = "AtomSound", meta = (ToolTip = "Set pitch in cent."))
	void SetPitch(float Pitch);

	/* AISACコントロール値の指定（名前指定） */
	UFUNCTION(BlueprintCallable, Category = "AtomSound", meta = (ToolTip = "Set Aisac by name."))
	void SetAisacByName(FString ControlName, float ControlValue);

	/* DEPRECATED - Use SetBusSendLevelByName instead. */
	UFUNCTION(BlueprintCallable, Category = "AtomSound", meta = (ToolTip = "Use SetBusSendLevelByName instead."))
	void SetBusSendLevel(int32 BusId, float Level);

	/* バスセンドレベルの設定 */
	UFUNCTION(BlueprintCallable, Category = "AtomSound", meta = (ToolTip = "Set bus send level."))
	void SetBusSendLevelByName(FString BusName, float Level);

	/* DEPRECATED - Use SetBusSendLevelOffsetByName instead. */
	UFUNCTION(BlueprintCallable, Category = "AtomSound", meta = (ToolTip = " Use SetBusSendLevelOffsetByName instead."))
	void SetBusSendLevelOffset(int32 BusId, float LevelOffset);

	/* バスセンドレベルの設定（オフセット指定） */
	UFUNCTION(BlueprintCallable, Category = "AtomSound", meta = (ToolTip = "Set bus send level offset."))
	void SetBusSendLevelOffsetByName(FString BusName, float LevelOffset);

	/* ブロックの遷移 */
	UFUNCTION(BlueprintCallable, Category = "AtomSound", meta = (ToolTip = "Set next block index."))
	void SetNextBlockIndex(int32 BlockIndex);

	/* セレクタラベルの設定 */
	UFUNCTION(BlueprintCallable, Category = "AtomSound", meta = (ToolTip = "Set selector label."))
	void SetSelectorLabel(FString Selector, FString Label);

	/** キューの再生時刻を取得します。単位は秒です。 */
	UFUNCTION(BlueprintCallable, Category="AtomSound", meta = (ToolTip = "Get current playback time."))
	float GetTime();

	/** キューの再生位置を取得します。単位は秒です。 */
	UFUNCTION(BlueprintCallable, Category="AtomSound", meta = (ToolTip = "Get current playback position."))
	float GetSequencePosition();

	/* AtomExプレーヤハンドルの取得 */
	CriAtomExPlayerHn GetAtomExPlayer();

	/* AtomEx3dSourceハンドルの取得 */
	CriAtomEx3dSourceHn GetAtomEx3dSource();

	/* コンポーネント単位で距離減衰を設定するかどうか */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Attenuation, meta = (ToolTip = "Attenuation settings for asset."))
	uint32 bOverrideAttenuation : 1;

	/* 距離減衰設定（距離減衰アセットを使用） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Attenuation, meta = (EditCondition = "!bOverrideAttenuation", ToolTip = "Enable attenuation settings."))
	class USoundAttenuation* AttenuationSettings;

	/* 距離減衰設定（コンポーネント単位の上書き設定） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Attenuation, meta = (EditCondition = "bOverrideAttenuation", ToolTip = "Attenuation overrides."))
	struct FSoundAttenuationSettings AttenuationOverrides;

	/* C++呼び出し専用 */
	void SetSound(USoundAtomCueSheet* InCueSheet, FString InCueName);

public:
	// Begin UObject interface.
	virtual void PostInitProperties() override;
	virtual void BeginDestroy() override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	virtual void PostLoad() override;
	// End UObject interface.

	// Begin ActorComponent interface 
#if WITH_EDITORONLY_DATA
	virtual void OnRegister() override;
#endif
	virtual void OnUnregister() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;
	// End ActorComponent interface

	// Begin USceneComponent Interface
	virtual void Activate(bool bReset = false) override;
	virtual void Deactivate() override;
	virtual void OnUpdateTransform(EUpdateTransformFlags UpdateTransformFlags, ETeleportType Teleport = ETeleportType::None) override;
#if WITH_EDITOR
	/* 音源表示関連のコンポーネントを破棄する */
	virtual void OnAttachmentChanged() override;
#endif
	// End USceneComponent Interface

	/* AttenuationSettingsの取得 */
	const FSoundAttenuationSettings* GetAttenuationSettingsToApply() const;

	uint64 GetAtomComponentID() const { return AtomComponentID; }

	void UpdateInteriorSettings(bool bFullUpdate);

private:
	static uint64 AtomComponentIDCounter;
	uint64 AtomComponentID;

	/* 音源 */
	static const uint32 PLAYER_WORK_SIZE = 1536;
	static const uint32 SOURCE_WORK_SIZE = 512;
	uint8 PlayerWork[PLAYER_WORK_SIZE];
	uint8 SourceWork[SOURCE_WORK_SIZE];
	CriAtomExPlayerHn Player;
	CriAtomEx3dSourceHn Source;

	/* 距離倍率 */
	float DistanceFactor;

	/* OcclusionのCollision判定などを行うための構造体 */
	FAtomActiveSound AtomActiveSound;
	float Filter_Frequency;

	/* キューシート */
	class USoundAtomCueSheet* CueSheet;

	/* キュー名 */
	FString CueName;

	/* 再生ID */
	uint32 PlaybackId;

	/* リソースの確保 */
	void AllocateResource();

	/* リソースの解放 */
	void ReleaseResource();

	/* トランスフォームの更新 */
	void UpdateTransform(void);

	/* 距離減衰の適用 */
	void ApplyAttenuation(float& Volume, float& FilterFrequency);

#if WITH_EDITORONLY_DATA
	/* スピーカーアイコン表示 */
	void UpdateSpriteTexture();

	UPROPERTY(transient, meta = (ToolTip = "Maximum distance for Sphere Component."))
		class UDrawSphereComponent* MaxDistanceSphereComponent;

	UPROPERTY(transient, meta = (ToolTip = "Minimum distance for Sphere Component."))
		class UDrawSphereComponent* MinDistanceSphereComponent;
#endif

#if WITH_EDITOR
	/* 音源範囲の表示（距離減衰上書き時用） */
	void DrawDebugShape();

	/* プレビュー終了検知用デリゲート */
	void OnWorldCleanedUp(UWorld* World, bool bSessionEnded, bool bCleanupResources);

	/* デリゲートハンドル */
	FDelegateHandle OnWorldCleanedUpHandle;
#endif
};

/***************************************************************************
 *      関数宣言
 *      Prototype Functions
 ***************************************************************************/

/* --- end of file --- */
