/****************************************************************************
 *
 * CRI Middleware SDK
 *
 * Copyright (c) 2013-2017 CRI Middleware Co., Ltd.
 *
 * Library  : CRIWARE plugin for Unreal Engine 4
 * Module   : Listener
 * File     : AtomListener.cpp
 *
 ****************************************************************************/

/***************************************************************************
 *      インクルードファイル
 *      Include files
 ***************************************************************************/
/* モジュールヘッダ */
#include "AtomListener.h"

/* CRIWAREプラグインヘッダ */
#include "CriWareRuntimePrivatePCH.h"

/* Unreal Engine 4関連ヘッダ */
#include "GameFramework/PlayerController.h"
#include "HeadMountedDisplay.h"
#include "HeadMountedDisplayFunctionLibrary.h"

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
/* コンストラクタ */
FAtomListener::FAtomListener(CriAtomEx3dListenerHn InListenerHandle, float InDistanceFactor)
{
	/* リスナハンドルを保存 */
	ListenerHandle = InListenerHandle;

	/* 距離係数の保存 */
	DistanceFactor = InDistanceFactor;

	/* リスナを自動で操作するよう設定 */
	bAutoUpdateEnabled = true;

	/* リスナ位置の初期化 */
	ListenerLocation = FVector::ZeroVector;
	ListenerRotation = FRotator::ZeroRotator;
}

/* 有効／無効の切り替え */
void FAtomListener::SetAutoUpdateEnabled(bool bEnabled)
{
	bAutoUpdateEnabled = bEnabled;
}

/* リスナ位置の指定 */
void FAtomListener::SetListenerLocation(FVector Location)
{
	ListenerLocation = Location;
}

/* リスナの向きの指定 */
void FAtomListener::SetListenerRotation(FRotator Rotation)
{
	ListenerRotation = Rotation;
	ListenerRotation_Quat = ListenerRotation.Quaternion();
}

/* リスナ位置の取得 */
FVector FAtomListener::GetListenerLocation()
{
	return ListenerLocation;
}

/* リスニングポイントの取得 */
FVector FAtomListener::GetListeningPoint()
{
	CriAtomExVector pos;
	FVector ListeningPoint;
	float level;

	/* フォーカスポイントの取得 */
	criAtomEx3dListener_GetFocusPoint(ListenerHandle, &pos);

	/* Distance Focus Levelの取得 */
	level = criAtomEx3dListener_GetDistanceFocusLevel(ListenerHandle);

	/* リスニングポイントの計算 */
	ListeningPoint.X = pos.x / DistanceFactor;
	ListeningPoint.Y = pos.y / DistanceFactor;
	ListeningPoint.Z = pos.z / DistanceFactor;

	/* リスニングポイントの計算 */
	ListeningPoint = ListeningPoint * level + ListenerLocation * (1.0f - level);

	return ListeningPoint;
}

void FAtomListener::Tick(float DeltaTime)
{
#if WITH_EDITOR
	/* プレビュー実行中かどうかチェック */
	bool bIsDebugging = (GEditor ? (GEditor->PlayWorld != nullptr) : false);
	if ((GIsEditor != false) && (bIsDebugging == false)) {
		/* エディタ上ではプレビュー中のみリスナの更新を行う */
		return;
	}
#endif

	/* 初期化済みかどうかチェック */
	if ((criAtomEx_IsInitialized() == CRI_FALSE) || (ListenerHandle == NULL)) {
		return;
	}

	/* リスナ座標を自動更新するかどうかのチェック */
	if (bAutoUpdateEnabled != false) {
		APlayerController* PlayerController = GWorld->GetFirstPlayerController();
		if (PlayerController != NULL) {
			if (UHeadMountedDisplayFunctionLibrary::IsHeadMountedDisplayEnabled()) {
				/* VR Modeの場合HMDのポジションと角度を加える */
				PlayerController->GetPlayerViewPoint(ListenerLocation, ListenerRotation);
				FVector HMDLocation;
				FRotator HMDRotation;
				UHeadMountedDisplayFunctionLibrary::GetOrientationAndPosition(HMDRotation, HMDLocation);
				ListenerLocation += HMDLocation;
				ListenerRotation += HMDRotation;
			} else {
				/* プレーヤが存在する場合はプレーヤのビューポートを取得 */
				FVector FrontDir, RightDir, TopDir;
				PlayerController->GetAudioListenerPosition(ListenerLocation, FrontDir, RightDir);
				ListenerRotation = FrontDir.Rotation();
			}
		} else {
#if WITH_EDITOR
			/* エディタ使用時はAtomPreviewer側でカメラのビューポートをセット */
#else
			/* プレーヤが存在しない場合はビューポートをリセット */
			ListenerLocation = FVector::ZeroVector;
			ListenerRotation = FRotator::ZeroRotator;
#endif
		}
		ListenerRotation_Quat = ListenerRotation.Quaternion();
	}

	/* リスナの座標を設定 */
	CriAtomExVector pos, front, top;
	pos.x = ListenerLocation.X * DistanceFactor;
	pos.y = ListenerLocation.Y * DistanceFactor;
	pos.z = ListenerLocation.Z * DistanceFactor;
	criAtomEx3dListener_SetPosition(ListenerHandle, &pos);
	
	/* リスナの向きを設定 */
	/* クオータニオンから前方ベクトルと上方ベクトルを取得 */
	FVector FrontVector = ListenerRotation_Quat.GetForwardVector();
	FVector TopVector = ListenerRotation_Quat.GetUpVector();
	front.x = FrontVector.X;
	front.y = FrontVector.Y;
	front.z = FrontVector.Z;
	top.x = TopVector.X;
	top.y = TopVector.Y;
	top.z = TopVector.Z;
	criAtomEx3dListener_SetOrientation(ListenerHandle, &front, &top);

	/* リスナの更新 */
	criAtomEx3dListener_Update(ListenerHandle);

	/* サーバ処理の実行 */
	criAtomEx_ExecuteMain();
}

bool FAtomListener::IsTickable() const
{
	return true;
}

TStatId FAtomListener::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(FAtomListener, STATGROUP_Tickables);
}

/***************************************************************************
 *      関数定義
 *      Function Definition
 ***************************************************************************/

/* --- end of file --- */
