/****************************************************************************
 *
 * CRI Middleware SDK
 *
 * Copyright (c) 2017 CRI Middleware Co., Ltd.
 *
 * Library  : CRIWARE plugin for Unreal Engine 4
 * Module   : Movie Scene Atom Section
 * File     : MovieSceneAtomSection.cpp
 *
 ****************************************************************************/

/***************************************************************************
 *      インクルードファイル
 *      Include files
 ***************************************************************************/
/* モジュールヘッダ */
#include "Sections/MovieSceneAtomSection.h"

/* CRIWAREプラグインヘッダ */
#include "Evaluation/MovieSceneAtomTemplate.h"

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
UMovieSceneAtomSection::UMovieSceneAtomSection(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	Sound = nullptr;
	StartOffset = 0.f;
	SoundVolume.SetDefaultValue(1.f);
	PitchMultiplier.SetDefaultValue(1.f);
#if WITH_EDITORONLY_DATA
	bShowIntensity = false;
#endif
	bSuppressSubtitles = false;

	EvalOptions.EnableAndSetCompletionMode(EMovieSceneCompletionMode::RestoreState);
}

FMovieSceneEvalTemplatePtr UMovieSceneAtomSection::GenerateTemplate() const
{
	return FMovieSceneAtomSectionTemplate(*this);
}

TRange<float> UMovieSceneAtomSection::GetAtomCueRange() const
{
	if (!Sound) {
		return TRange<float>::Empty();
	}

	float AtomRangeStartTime = GetStartTime();
	if (StartOffset < 0) {
		AtomRangeStartTime += FMath::Abs(StartOffset);
	}

	/* 音声データの長さを取得 */
	int32 SoundDuration = Sound->GetLength();

	/* ループデータかどうか判定 */
	float DurationToUse;
	if (SoundDuration != -1) {
		DurationToUse = (float)SoundDuration / 1000.0f;
	} else {
		DurationToUse = (float)MAX_uint32 / 1000.0f;
	}

	float AtomRangeEndTime = FMath::Min(AtomRangeStartTime + DurationToUse * PitchMultiplier.GetDefaultValue(), GetEndTime());

	return TRange<float>(AtomRangeStartTime, AtomRangeEndTime);
}

void UMovieSceneAtomSection::PostLoad()
{
	Super::PostLoad();
}

void UMovieSceneAtomSection::MoveSection(float DeltaTime, TSet<FKeyHandle>& KeyHandles)
{
	Super::MoveSection(DeltaTime, KeyHandles);

	SoundVolume.ShiftCurve(DeltaTime, KeyHandles);
	PitchMultiplier.ShiftCurve(DeltaTime, KeyHandles);
}

void UMovieSceneAtomSection::DilateSection(float DilationFactor, float Origin, TSet<FKeyHandle>& KeyHandles)
{
	Super::DilateSection(DilationFactor, Origin, KeyHandles);

	SoundVolume.ScaleCurve(Origin, DilationFactor, KeyHandles);
	PitchMultiplier.ScaleCurve(Origin, DilationFactor, KeyHandles);
}

UMovieSceneSection* UMovieSceneAtomSection::SplitSection(float SplitTime)
{
	float NewOffset = StartOffset + (SplitTime - GetStartTime());

	UMovieSceneSection* NewSection = Super::SplitSection(SplitTime);
	if (NewSection != nullptr) {
		UMovieSceneAtomSection* NewAtomSection = Cast<UMovieSceneAtomSection>(NewSection);
		NewAtomSection->StartOffset = NewOffset;
	}
	return NewSection;
}

void UMovieSceneAtomSection::GetKeyHandles(TSet<FKeyHandle>& OutKeyHandles, TRange<float> TimeRange) const
{
	if (!TimeRange.Overlaps(GetRange())) {
		return;
	}

	for (auto It(SoundVolume.GetKeyHandleIterator()); It; ++It) {
		float Time = SoundVolume.GetKeyTime(It.Key());
		if (TimeRange.Contains(Time)) {
			OutKeyHandles.Add(It.Key());
		}
	}

	for (auto It(PitchMultiplier.GetKeyHandleIterator()); It; ++It) {
		float Time = PitchMultiplier.GetKeyTime(It.Key());
		if (TimeRange.Contains(Time)) {
			OutKeyHandles.Add(It.Key());
		}
	}
}

void UMovieSceneAtomSection::GetSnapTimes(TArray<float>& OutSnapTimes, bool bGetSectionBorders) const
{
	Super::GetSnapTimes(OutSnapTimes, bGetSectionBorders);

	// @todo Sequencer handle snapping for time dilation
	// @todo Don't add redundant times (can't use AddUnique due to floating point equality issues)
}

/***************************************************************************
 *      関数定義
 *      Function Definition
 ***************************************************************************/

#undef LOCTEXT_NAMESPACE

/* --- end of file --- */
