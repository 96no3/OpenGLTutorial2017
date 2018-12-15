/****************************************************************************
 *
 * CRI Middleware SDK
 *
 * Copyright (c) 2013-2016 CRI Middleware Co., Ltd.
 *
 * Library  : CRIWARE plugin for Unreal Engine 4
 * Module   : Precompiled header for CriWareEditor
 * File     : CriWareEditorPrivatePCH.h
 *
 ****************************************************************************/

/* 多重定義防止 */
#pragma once

/***************************************************************************
 *      インクルードファイル
 *      Include files
 ***************************************************************************/
#include "AssetData.h"
#include "AssetToolsModule.h"
#include "TextFilter.h"
#include "Internationalization.h"

DECLARE_LOG_CATEGORY_EXTERN(LogCriWareEditor, Log, All);
#define CRIWARE_DEFINE_COMPATIBLE_MACRO
#include "CriWareApi.h"
#include "CriWareLocalization.h"

/* --- end of file --- */
