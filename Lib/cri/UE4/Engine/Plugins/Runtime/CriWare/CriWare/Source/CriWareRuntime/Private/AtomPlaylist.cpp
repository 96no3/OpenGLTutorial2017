/****************************************************************************
 *
 * CRI Middleware SDK
 *
 * Copyright (c) 2018 CRI Middleware Co., Ltd.
 *
 * Library  : CRIWARE plugin for Unreal Engine 4
 * Module   : Atom Playlist
 * File     : AtomPlaylist.cpp
 *
 ****************************************************************************/

/***************************************************************************
 *      インクルードファイル
 *      Include files
 ***************************************************************************/
/* モジュールヘッダ */
#include "AtomPlaylist.h"

/* CRIWAREプラグインヘッダ */
#include "CriWareRuntimePrivatePCH.h"
#include "SoundAtomCueSheet.h"

/***************************************************************************
 *      定数マクロ
 *      Macro Constants
 ***************************************************************************/
#define LOCTEXT_NAMESPACE "AtomPlaylist"

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
FAtomPlaylist::FAtomPlaylist()
{
	Sounds.Reset();
	Offset = 0;
}

FAtomPlaylist::~FAtomPlaylist()
{
	/* 注意）リストに残ったアイテムの参照を解除する必要あり。 */
	Reset();
}

void FAtomPlaylist::Add(USoundAtomCue* Sound)
{
	/* 複数スレッドからのリストアクセスを排他制御 */
	FScopeLock Lock(&CriticalSection);

	/* キュー名の取得 */
	FString CueName = Sound->CueName;
	if (CueName.Len() <= 0) {
		return;
	}

	/* キューシートハンドルの取得 */
	USoundAtomCueSheet* CueSheet = Sound->CueSheet;
	CriAtomExAcbHn AcbHn = ((CueSheet != nullptr) ? CueSheet->GetAcbHn() : nullptr);

	/* 再生するキューに含まれる波形データの情報を取得 */
	CriAtomExWaveformInfo WaveInfo;
	criAtomExAcb_GetWaveformInfoByName(AcbHn, TCHAR_TO_UTF8(*CueName), &WaveInfo);

	/* 波形データがストリーム再生用かどうかをチェック */
	CriAtomAwbHn AwbHn;
	if (WaveInfo.streaming_flag == CRI_FALSE) {
		/* ACBハンドルからオンメモリ再生用のAWBハンドルを取得 */
		AwbHn = criAtomExAcb_GetOnMemoryAwbHandle(AcbHn);
	} else {
		/* ACBハンドルからストリーム再生用のAWBハンドルを取得 */
		AwbHn = criAtomExAcb_GetStreamingAwbHandle(AcbHn);
	}

	/* リストアイテムの確保 */
	FAtomPlaylistItem* Item = new FAtomPlaylistItem();
	if (Item == nullptr) {
		UE_LOG(LogCriWareRuntime, Error, TEXT("Failed to allocate list item."));
		return;
	}

	/* リスト登録中のGCを回避 */
	Sound->CueSheet->AddRef();

	/* リストに追加 */
	Item->Sound = Sound;
	Item->AwbHn = AwbHn;
	Item->WaveId = WaveInfo.wave_id;
	Sounds.Add(Item);
}

void FAtomPlaylist::RemoveAt(int32 Index)
{
	/* 複数スレッドからのリストアクセスを排他制御 */
	FScopeLock Lock(&CriticalSection);

	/* 指定範囲のチェック */
	if (!Sounds.IsValidIndex(Index)) {
		return;
	}

	/* リストアイテムの取得 */
	FAtomPlaylistItem* Item = Sounds[Index];

	/* アイテムの登録を解除 */
	Sounds.RemoveAt(Index);

	/* キューシートの参照を解除 */
	Item->Sound->CueSheet->Release();

	/* アイテムの破棄 */
	delete Item;

	/* 再生位置の更新 */
	if (Index < Offset) {
		/* 手前のアイテムが破棄された場合は再生位置を読み替える */
		Offset--;
	}
}

int32 FAtomPlaylist::Num()
{
	/* 複数スレッドからのリストアクセスを排他制御 */
	FScopeLock Lock(&CriticalSection);

	return Sounds.Num();
}

void FAtomPlaylist::Reset()
{
	/* 複数スレッドからのリストアクセスを排他制御 */
	FScopeLock Lock(&CriticalSection);

	/* 再生位置のリセット */
	Offset = 0;

	/* リストに残っているアイテムを破棄 */
	while (Num() > 0) {
		RemoveAt(0);
	}

	/* リストのクリア */
	Sounds.Reset();
}

const FAtomPlaylistItem* FAtomPlaylist::Get(int32 Index)
{
	/* 複数スレッドからのリストアクセスを排他制御 */
	FScopeLock Lock(&CriticalSection);

	/* 指定範囲のチェック */
	if (!Sounds.IsValidIndex(Index)) {
		return nullptr;
	}

	return Sounds[Index];
}

/***************************************************************************
 *      関数定義
 *      Function Definition
 ***************************************************************************/

#undef LOCTEXT_NAMESPACE

/* --- end of file --- */
