/****************************************************************************
 *
 * CRI Middleware SDK
 *
 * Copyright (c) 2013-2017 CRI Middleware Co., Ltd.
 *
 * Library  : CRIWARE plugin for Unreal Engine 4
 * Module   : Listener
 * File     : AtomListener.h
 *
 ****************************************************************************/

/* 多重定義防止 */
#pragma once

/***************************************************************************
 *      インクルードファイル
 *      Include files
 ***************************************************************************/
/* Unreal Engine 4関連ヘッダ */
#include "Tickable.h"

/* CRIWAREプラグインヘッダ */
#include "CriWareApi.h"

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
class FAtomListener : public FTickableGameObject
{
public:
	/* コンストラクタ */
	FAtomListener(CriAtomEx3dListenerHn InListenerHandle, float InDistanceFactor);

	/* 有効／無効の切り替え */
	void SetAutoUpdateEnabled(bool bEnabled);

	/* リスナ位置の指定 */
	void SetListenerLocation(FVector Location);

	/* リスナの向きの指定 */
	void SetListenerRotation(FRotator Rotation);

	/* リスナ位置の取得 */
	FVector GetListenerLocation();

	/* リスニングポイント（フォーカスポイントを加味した位置）の取得 */
	FVector GetListeningPoint();

	// Begin FTickableObjectBase Interface
	virtual void Tick(float DeltaTime) override;
	// End FTickableObjectBase Interface

private:
	/* リスナ */
	CriAtomEx3dListenerHn ListenerHandle;

	/* 距離係数 */
	float DistanceFactor;

	/* リスナを自動で操作するかどうか */
	bool bAutoUpdateEnabled;

	/* リスナの位置 */
	FVector ListenerLocation;

	/* リスナの向き */
	FRotator ListenerRotation;
	FQuat ListenerRotation_Quat;

	// Begin FTickableObjectBase Interface
	virtual bool IsTickable() const override;
	virtual TStatId GetStatId() const override;
	// End FTickableObjectBase Interface
};

/***************************************************************************
 *      関数宣言
 *      Prototype Functions
 ***************************************************************************/

/* --- end of file --- */
