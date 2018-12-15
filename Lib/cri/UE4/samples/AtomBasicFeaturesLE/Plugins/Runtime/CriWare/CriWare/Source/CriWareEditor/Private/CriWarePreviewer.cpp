/****************************************************************************
 *
 * CRI Middleware SDK
 *
 * Copyright (c) 2013-2017 CRI Middleware Co., Ltd.
 *
 * Library  : CRIWARE plugin for Unreal Engine 4
 * Module   : Previewer
 * File     : CriWarePreviewer.cpp
 *
 ****************************************************************************/

/***************************************************************************
 *      インクルードファイル
 *      Include files
 ***************************************************************************/
/* モジュールヘッダ */
#include "CriWarePreviewer.h"

/* CRIWAREプラグインヘッダ */
#include "CriWareEditorPrivatePCH.h"
#include "AtomPreviewer.h"

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
/* FCriWarePreviewerStatics共有用 */
typedef TSharedPtr<class FCriWarePreviewerStatics> FCriWarePreviewerStaticsPtr;
typedef TSharedRef<class FCriWarePreviewerStatics> FCriWarePreviewerStaticsRef;

/***************************************************************************
 *      変数宣言
 *      Prototype Variables
 ***************************************************************************/

/***************************************************************************
 *      クラス宣言
 *      Prototype Classes
 ***************************************************************************/
class FCriWarePreviewerStatics
{
public:
	/* コンストラクタ */
	FCriWarePreviewerStatics();

	/* デストラクタ */
	~FCriWarePreviewerStatics();

	/* CriWarePreviewerStaticsの取得 */
	static FCriWarePreviewerStaticsRef GetCriWarePreviewerStatics();

	/* 初期化処理 */
	void Initialize();

	/* 終了処理 */
	void Finalize();

	/* エディタプレビュー用 */
	void PlayPreviewSound(USoundAtomCue* Sound);
	bool IsPreviewing(USoundAtomCue* Sound);
	void StopPreviewing();

	/* AtomPreviewer */
	FAtomPreviewer* AtomPreviewer;

private:
	/* 共有されるCriWarePreviewerStatics */
	static FCriWarePreviewerStaticsPtr CriWarePreviewerStaticsSingleton;

	/* 初期化カウンタ */
	int32 InitializationCount;
};

/***************************************************************************
 *      関数宣言
 *      Prototype Functions
 ***************************************************************************/

/***************************************************************************
 *      変数定義
 *      Variable Definition
 ***************************************************************************/
FCriWarePreviewerStaticsPtr FCriWarePreviewerStatics::CriWarePreviewerStaticsSingleton;

/***************************************************************************
 *      クラス定義
 *      Class Definition
 ***************************************************************************/
void UCriWarePreviewer::PostInitProperties()
{
	Super::PostInitProperties();

	/* 初期化処理の実行 */
	FCriWarePreviewerStatics::GetCriWarePreviewerStatics()->Initialize();
}

void UCriWarePreviewer::BeginDestroy()
{
	Super::BeginDestroy();

	/* 終了処理の実行 */
	FCriWarePreviewerStatics::GetCriWarePreviewerStatics()->Finalize();
}

void UCriWarePreviewer::PlayPreviewSound(USoundAtomCue* Sound)
{
	FCriWarePreviewerStatics::GetCriWarePreviewerStatics()->PlayPreviewSound(Sound);
}

bool UCriWarePreviewer::IsPreviewing(USoundAtomCue* Sound)
{
	return FCriWarePreviewerStatics::GetCriWarePreviewerStatics()->IsPreviewing(Sound);
}

void UCriWarePreviewer::StopPreviewing()
{
	FCriWarePreviewerStatics::GetCriWarePreviewerStatics()->StopPreviewing();
}

/* コンストラクタ */
FCriWarePreviewerStatics::FCriWarePreviewerStatics()
{
	InitializationCount = 0;
	AtomPreviewer = NULL;
}

/* デストラクタ */
FCriWarePreviewerStatics::~FCriWarePreviewerStatics()
{
}

/* FCriWarePreviewerStaticsの取得 */
FCriWarePreviewerStaticsRef FCriWarePreviewerStatics::GetCriWarePreviewerStatics()
{
	/* 実体の有無をチェック */
	if (!FCriWarePreviewerStatics::CriWarePreviewerStaticsSingleton.IsValid()) {
		/* 実体がなければ確保 */
		FCriWarePreviewerStatics::CriWarePreviewerStaticsSingleton = MakeShareable(new FCriWarePreviewerStatics);
	}

	/* 参照ポインタを返す */
	return FCriWarePreviewerStatics::CriWarePreviewerStaticsSingleton.ToSharedRef();
}

/* 初期化処理 */
void FCriWarePreviewerStatics::Initialize()
{
	/* 初期化カウンタの更新 */
	InitializationCount++;
	if (InitializationCount != 1) {
		return;
	}

	/* AtomPreviewerの作成 */
	AtomPreviewer = new FAtomPreviewer();
}

/* 終了処理 */
void FCriWarePreviewerStatics::Finalize()
{
	/* 初期化カウンタの更新 */
	InitializationCount--;
	if (InitializationCount != 0) {
		return;
	}

	/* AtomPreviewerの破棄 */
	if (AtomPreviewer != NULL) {
		AtomPreviewer->ClearPreviewComponents();
		delete AtomPreviewer;
		AtomPreviewer = NULL;
	}
}

void FCriWarePreviewerStatics::PlayPreviewSound(USoundAtomCue* Sound)
{
	if (AtomPreviewer != nullptr) {
		AtomPreviewer->PlayPreviewSound(Sound);
	}
}

bool FCriWarePreviewerStatics::IsPreviewing(USoundAtomCue* Sound)
{
	if (AtomPreviewer == nullptr) {
		return false;
	}

	UAtomComponent* AtomComponent = AtomPreviewer->GetPreviewAtomComponent();
	if (AtomComponent == nullptr) {
		return false;
	}

	if (AtomComponent->Sound != Sound) {
		return false;
	}

	return AtomComponent->IsPlaying();
}

void FCriWarePreviewerStatics::StopPreviewing()
{
	if (AtomPreviewer != nullptr) {
		AtomPreviewer->ResetPreviewAtomComponent();
	}
}

/***************************************************************************
 *      関数定義
 *      Function Definition
 ***************************************************************************/

/* --- end of file --- */
