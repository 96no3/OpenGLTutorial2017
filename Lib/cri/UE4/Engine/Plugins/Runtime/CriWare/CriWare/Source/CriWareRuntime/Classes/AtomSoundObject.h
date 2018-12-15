/****************************************************************************
 *
 * CRI Middleware SDK
 *
 * Copyright (c) 2014-2017 CRI Middleware Co., Ltd.
 *
 * Library  : CRIWARE plugin for Unreal Engine 4
 * Module   : Atom SoundObject
 * File     : AtomSoundObject.h
 *
 ****************************************************************************/

/* 多重定義防止 */
#pragma once

/***************************************************************************
 *      インクルードファイル
 *      Include files
 ***************************************************************************/
/* Unreal Engine 4関連ヘッダ */
#include "Engine/DataAsset.h"

/* CRIWAREプラグインヘッダ */
#include "CriWareApi.h"

/* モジュールヘッダ */
#include "AtomSoundObject.generated.h"

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

/***************************************************************************
 *      変数宣言
 *      Prototype Variables
 ***************************************************************************/

/***************************************************************************
 *      クラス宣言
 *      Prototype Classes
 ***************************************************************************/
UCLASS(MinimalAPI, BlueprintType, meta=(ToolTip = "AtomSoundObject class."))
class UAtomSoundObject : public UDataAsset
{
	GENERATED_BODY()

public:
	UAtomSoundObject(const FObjectInitializer& ObjectInitializer);

	/* ボイスリミットのスコープを分けるかどうか */
	UPROPERTY(EditAnywhere, Category="Config", meta = (ToolTip = "Enable voice limit scope."))
	bool EnableVoiceLimitScope;

	/* カテゴリキューリミットのスコープを分けるかどうか */
	UPROPERTY(EditAnywhere, Category="Config", meta = (ToolTip = "Enable category Cue limit scope."))
	bool EnableCategoryCueLimitScope;

	/* SoundObjectハンドルの取得 */
	CriAtomExSoundObjectHn GetSoundObjectHandle();

public:
	// Begin UObject interface.
	virtual void BeginDestroy() override;
	virtual void Serialize(FArchive& Ar) override;
	virtual void PostLoad() override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	// End of UObject interface

private:
	/* SoundObjectの確保 */
	void AllocateSoundObject();

	/* SoundObjectの解放 */
	void ReleaseSoundObject();

	/* SoundObjectの再確保 */
	void UpdateSoundObject();

	/* SoundObject */
	CriAtomExSoundObjectHn SoundObject;
};

/***************************************************************************
 *      関数宣言
 *      Prototype Functions
 ***************************************************************************/

/* --- end of file --- */
