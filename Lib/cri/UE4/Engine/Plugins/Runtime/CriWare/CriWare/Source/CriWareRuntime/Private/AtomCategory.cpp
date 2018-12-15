/****************************************************************************
 *
 * CRI Middleware SDK
 *
 * Copyright (c) 2016-2017 CRI Middleware Co., Ltd.
 *
 * Library  : CRIWARE plugin for Unreal Engine 4
 * Module   : Atom Category
 * File     : AtomCategory.cpp
 *
 ****************************************************************************/

/***************************************************************************
 *      インクルードファイル
 *      Include files
 ***************************************************************************/
/* モジュールヘッダ */
#include "AtomCategory.h"

/* CRIWAREプラグインヘッダ */
#include "CriWareRuntimePrivatePCH.h"

/***************************************************************************
 *      定数マクロ
 *      Macro Constants
 ***************************************************************************/
#define LOCTEXT_NAMESPACE "AtomCategory"

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
void UAtomCategory::SetVolumeByName(FString CategoryName, float Volume)
{
	criAtomExCategory_SetVolumeByName(TCHAR_TO_UTF8(*CategoryName), Volume);
}

float UAtomCategory::GetVolumeByName(FString CategoryName)
{
	return criAtomExCategory_GetVolumeByName(TCHAR_TO_UTF8(*CategoryName));
}

void UAtomCategory::PauseByName(FString CategoryName, bool bPause)
{
	criAtomExCategory_PauseByName(TCHAR_TO_UTF8(*CategoryName), (CriBool)bPause);
}

bool UAtomCategory::IsPausedByName(FString CategoryName)
{
	CriBool result = criAtomExCategory_IsPausedByName(TCHAR_TO_UTF8(*CategoryName));
	return (result != CRI_FALSE);
}

void UAtomCategory::SetAisacControlByName(FString CategoryName, FString AisacName, float Value)
{
	criAtomExCategory_SetAisacControlByName(TCHAR_TO_UTF8(*CategoryName), TCHAR_TO_UTF8(*AisacName), Value);
}

void UAtomCategory::StopByName(FString CategoryName)
{
	criAtomExCategory_StopByName(TCHAR_TO_UTF8(*CategoryName));
}

/***************************************************************************
 *      関数定義
 *      Function Definition
 ***************************************************************************/
#undef LOCTEXT_NAMESPACE

/* --- end of file --- */
