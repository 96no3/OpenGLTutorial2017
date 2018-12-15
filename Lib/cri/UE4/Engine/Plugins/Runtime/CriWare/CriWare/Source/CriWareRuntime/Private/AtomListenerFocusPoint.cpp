/****************************************************************************
 *
 * CRI Middleware SDK
 *
 * Copyright (c) 2015-2017 CRI Middleware Co., Ltd.
 *
 * Library  : CRIWARE plugin for Unreal Engine 4
 * Module   : Lisntener Focus Point
 * File     : AtomListenerFocusPoint.cpp
 *
 ****************************************************************************/

/***************************************************************************
 *      インクルードファイル
 *      Include files
 ***************************************************************************/
/* モジュールヘッダ */
#include "AtomListenerFocusPoint.h"

/* CRIWAREプラグインヘッダ */
#include "CriWareRuntimePrivatePCH.h"

/***************************************************************************
 *      定数マクロ
 *      Macro Constants
 ***************************************************************************/

#define LOCTEXT_NAMESPACE "AtomListenerFocusPoint"

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

/***************************************************************************
 *      クラス定義
 *      Class Definition
 ***************************************************************************/
UAtomListenerFocusPoint::UAtomListenerFocusPoint(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
#if WITH_EDITOR
	/* Localization of unreal properties metadata with LOCTEXT markups and reflection */
	CRI_LOCCLASS(GetClass());
#endif

	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	bAutoActivate = true;

	/* フォーカスポイントの位置を更新するため、OnUpdateTransformを呼び出すよう指定する */
	bWantsOnUpdateTransform = true;

	DistanceFocusLevel = 1.0f;
	DirectionFocusLevel = 0.0f;
	Listener = NULL;
	DistanceFactor = 1.0f;
}

void UAtomListenerFocusPoint::PostInitProperties()
{
	Super::PostInitProperties();

	/* デフォルトオブジェクトかどうかチェック */
	if (HasAnyFlags(RF_ClassDefaultObject)) {
		/* デフォルトオブジェクトの場合は何もしない */
		return;
	}

	/* リスナの取得 */
	Listener = UCriWareInitializer::GetListener();
	if (Listener == NULL) {
		return;
	}

	/* 距離係数の取得 */
	DistanceFactor = UCriWareInitializer::GetDistanceFactor();
}

void UAtomListenerFocusPoint::BeginDestroy()
{
	/* フォーカスポイントの無効化 */
	/* 注意）GC処理の順序によっては、ライブラリ終了処理後にここに来る可能性がある。 */
	if (criAtomEx_IsInitialized() != CRI_FALSE) {
		DisableFocusPoint();
	}

	Super::BeginDestroy();
}

#if WITH_EDITOR
void UAtomListenerFocusPoint::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	/* フォーカスポイントの更新 */
	EnableFocusPoint();
}
#endif

void UAtomListenerFocusPoint::Activate(bool bReset)
{
	Super::Activate(bReset);

	/* フォーカスポイントの有効化 */
	EnableFocusPoint();
}

void UAtomListenerFocusPoint::Deactivate()
{
	Super::Deactivate();

	/* フォーカスポイントの無効化 */
	DisableFocusPoint();
}

void UAtomListenerFocusPoint::OnUpdateTransform(EUpdateTransformFlags UpdateTransformFlags, ETeleportType Teleport)
{
	Super::OnUpdateTransform(UpdateTransformFlags, Teleport);

	/* デフォルトオブジェクトかどうかチェック */
	if (HasAnyFlags(RF_ClassDefaultObject)) {
		/* デフォルトオブジェクトの場合は何もしない */
		return;
	}

	/* アクティブかどうかチェック */
	if (bIsActive == false) {
		/* 非アクティブ時は何もしない */
		return;
	}

	/* フォーカスポイントの更新 */
	EnableFocusPoint();
}

void UAtomListenerFocusPoint::EnableFocusPoint()
{
	UpdateFocusPoint();
}

void UAtomListenerFocusPoint::DisableFocusPoint()
{
	if (Listener != NULL) {
		/* フォーカスポイントの座標をクリア */
		CriAtomExVector pos = { 0.0f, };
		criAtomEx3dListener_SetFocusPoint((CriAtomEx3dListenerHn)Listener, &pos);

		/* フォーカスレベルをクリア */
		criAtomEx3dListener_SetDistanceFocusLevel((CriAtomEx3dListenerHn)Listener, 0.0f);
		criAtomEx3dListener_SetDirectionFocusLevel((CriAtomEx3dListenerHn)Listener, 0.0f);
	}
}

void UAtomListenerFocusPoint::UpdateFocusPoint()
{
	if (Listener == NULL) {
		Listener = UCriWareInitializer::GetListener();
	}

	if (Listener != NULL) {
		/* フォーカスポイントの座標を設定 */
		const FVector SourcePosition = GetComponentLocation();
		CriAtomExVector pos;
		pos.x = SourcePosition.X * DistanceFactor;
		pos.y = SourcePosition.Y * DistanceFactor;
		pos.z = SourcePosition.Z * DistanceFactor;
		criAtomEx3dListener_SetFocusPoint((CriAtomEx3dListenerHn)Listener, &pos);

		/* フォーカスレベルを設定 */
		criAtomEx3dListener_SetDistanceFocusLevel((CriAtomEx3dListenerHn)Listener, DistanceFocusLevel);
		criAtomEx3dListener_SetDirectionFocusLevel((CriAtomEx3dListenerHn)Listener, DirectionFocusLevel);
	}
}

/***************************************************************************
 *      関数定義
 *      Function Definition
 ***************************************************************************/
#undef LOCTEXT_NAMESPACE

/* --- end of file --- */
