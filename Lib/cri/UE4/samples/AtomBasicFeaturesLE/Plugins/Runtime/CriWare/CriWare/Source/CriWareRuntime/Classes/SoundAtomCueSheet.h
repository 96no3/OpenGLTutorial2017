/****************************************************************************
 *
 * CRI Middleware SDK
 *
 * Copyright (c) 2013-2017 CRI Middleware Co., Ltd.
 *
 * Library  : CRIWARE plugin for Unreal Engine 4
 * Module   : Atom Sound Data (CueSheet)
 * File     : SoundAtomCueSheet.h
 *
 ****************************************************************************/

/* 多重定義防止 */
#pragma once

/***************************************************************************
 *      インクルードファイル
 *      Include files
 ***************************************************************************/
/* Unreal Engine 4関連ヘッダ */
#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "UObject/Object.h"
#include "UObject/ScriptMacros.h"
#include "Serialization/BulkData.h"
#include "EditorFramework/AssetImportData.h"

/* CRIWAREプラグインヘッダ */
#include "CriWareApi.h"

/* モジュールヘッダ */
#include "SoundAtomCueSheet.generated.h"

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
UCLASS(hidecategories=object, meta = (ToolTip = "SoundAtomCueSheet class."))
class CRIWARERUNTIME_API USoundAtomCueSheet : public UObject
{
	GENERATED_BODY()

public:
	USoundAtomCueSheet(const FObjectInitializer& ObjectInitializer);

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Instanced, Category = Reimport)
	class UAssetImportData* AssetImportData;
#endif

	/* ACBデータがuasset内に含まれているかどうか */
	UPROPERTY(Transient, VisibleAnywhere, Category = "AcbDataHandling", meta = (DisplayName = "Is ACB data contained", ToolTip = "Is ACB data contained."))
	bool Contains;

	/** (DEPRECATED) ACB file path is now stored in AssetImportData. */
	UPROPERTY()
	FString AcbFilePath;

	/* ACBデータ */
	FByteBulkData RawData;

	/* AWBファイルのフォルダパスを指定するかどうか */
	/* コンポーネント単位で距離減衰を設定するかどうか */
	UPROPERTY(EditAnywhere, Category = "AwbFileHandling", meta = (DisplayName = "Override AWB directory", ToolTip = "Load AWB file from the specified path. Ignore [Non-Asset Content Directory] setting."))
	bool bOverrideAwbDirectory;

	/* AWBファイルを配置したフォルダのパス */
	UPROPERTY(EditAnywhere, Category = "AwbFileHandling", meta = (DisplayName = "AWB directory", ToolTip = "Directory containing AWB file."))
	FDirectoryPath AwbDirectory;

	/* キューシート名 */
	UPROPERTY(VisibleAnywhere, Category = "Details", meta = (DisplayName = "CueSheet name", ToolTip = "CueSheet name."))
	FString CueSheetName;

	/* ACBのロード */
	UFUNCTION(BlueprintCallable, Category = "CRIWARE", meta = (ToolTip = "Load ACB."))
	static USoundAtomCueSheet* LoadAcb(FName AcbName);

	/* ACBのリリース */
	UFUNCTION(BlueprintCallable, Category="CRIWARE", meta = (ToolTip = "Release ACB."))
	static void ReleaseAcb(FName AcbName);

	/* DSPバス設定の適用 */
	UFUNCTION(BlueprintCallable, Category="CRIWARE", meta = (ToolTip = "Attach DSP bus setting."))
	static void AttachDspBusSetting(FString SettingName);

	/* DSPバス設定の無効化 */
	UFUNCTION(BlueprintCallable, Category="CRIWARE", meta = (ToolTip = "Detach DSP bus setting."))
	static void DetachDspBusSetting();

	/* DSPバススナップショットの適用 */
	UFUNCTION(BlueprintCallable, Category="CRIWARE", meta = (ToolTip = "Apply DSP bus setting."))
	static void ApplyDspBusSnapshot(FString SnapshotName, int32 Milliseconds);

	/* ACBの検索 */
	static USoundAtomCueSheet* Find(FName AcbName, FString PackagePath);

	/* 参照の追加 */
	void AddRef(void);

	/* 参照の削除 */
	void Release(void);

	/* ACBハンドルを返す */
	CriAtomExAcbHn GetAcbHn(void);

	// Begin UObject interface.
	virtual void PostInitProperties() override;
	virtual void Serialize(FArchive& Ar) override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	// End UObject interface.

private:
	/* 旧データの読み込み */
	void SerializeOldVersion(FArchive& Ar);

	/* アセットの更新 */
	/* 備考）Ver.1アセットをVer.2アセットに更新。 */
	void UpdateAsset();

	/* RawData展開先バッファ */
	uint8* ResourceData;

	/* ACBロード用ハンドル */
	CriAtomExAcbHn AcbHn;

	/* 参照カウント */
	int referenceCount;
};

/* AtomCueSheetアセットのバージョン管理用 */
struct FAtomCueSheetVersion
{
	/* バージョン番号 */
	static const int32 AssetVersion;

	/* GUID */
	static const FGuid AssetGuid;
};

/***************************************************************************
 *      関数宣言
 *      Prototype Functions
 ***************************************************************************/

/* --- end of file --- */
