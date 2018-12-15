/****************************************************************************
 *
 * CRI Middleware SDK
 *
 * Copyright (c) 2017 CRI Middleware Co., Ltd.
 *
 * Library  : CRIWARE plugin for Unreal Engine 4
 * Module   : Movie Scene Atom Track
 * File     : MovieSceneAtomTrack.cpp
 *
 ****************************************************************************/

/***************************************************************************
 *      インクルードファイル
 *      Include files
 ***************************************************************************/
/* モジュールヘッダ */
#include "Tracks/MovieSceneAtomTrack.h"

/* CRIWAREプラグインヘッダ */
#include "SoundAtomCue.h"

/* Unreal Engine 4関連ヘッダ */
#include "MovieScene.h"
#include "Sections/MovieSceneAtomSection.h"
#include "Kismet/GameplayStatics.h"
#include "Evaluation/MovieSceneSegment.h"
#include "Compilation/MovieSceneSegmentCompiler.h"
#include "Compilation/MovieSceneCompilerRules.h"

/***************************************************************************
 *      定数マクロ
 *      Macro Constants
 ***************************************************************************/
#define LOCTEXT_NAMESPACE "MovieSceneAtomTrack"

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
UMovieSceneAtomTrack::UMovieSceneAtomTrack(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
#if WITH_EDITORONLY_DATA
	TrackTint = FColor(93, 95, 136);
	RowHeight = 50;
#endif
}

const TArray<UMovieSceneSection*>& UMovieSceneAtomTrack::GetAllSections() const
{
	return AtomSections;
}

bool UMovieSceneAtomTrack::SupportsMultipleRows() const
{
	return true;
}

void UMovieSceneAtomTrack::RemoveAllAnimationData()
{
	// do nothing
}

bool UMovieSceneAtomTrack::HasSection(const UMovieSceneSection& Section) const
{
	return AtomSections.Contains(&Section);
}

void UMovieSceneAtomTrack::AddSection(UMovieSceneSection& Section)
{
	AtomSections.Add(&Section);
}

void UMovieSceneAtomTrack::RemoveSection(UMovieSceneSection& Section)
{
	AtomSections.Remove(&Section);
}

bool UMovieSceneAtomTrack::IsEmpty() const
{
	return AtomSections.Num() == 0;
}

TRange<float> UMovieSceneAtomTrack::GetSectionBoundaries() const
{
	TArray<TRange<float>> Bounds;
	for (int32 i = 0; i < AtomSections.Num(); ++i) {
		Bounds.Add(AtomSections[i]->GetRange());
	}
	return TRange<float>::Hull(Bounds);
}

void UMovieSceneAtomTrack::AddNewSound(USoundAtomCue* Sound, float Time)
{
	check(Sound);

	/* 音声データの長さを取得 */
	int32 SoundDuration = Sound->GetLength();

	/* ループデータかどうか判定 */
	float DurationToUse;
	if (SoundDuration != -1) {
		DurationToUse = (float)SoundDuration / 1000.0f;
	} else {
		DurationToUse = (float)MAX_uint32 / 1000.0f;
	}

	// add the section
	UMovieSceneAtomSection* NewSection = NewObject<UMovieSceneAtomSection>(this);
	NewSection->InitialPlacement(AtomSections, Time, Time + DurationToUse, SupportsMultipleRows());
	NewSection->SetSound(Sound);

	AtomSections.Add(NewSection);
}

bool UMovieSceneAtomTrack::IsAMasterTrack() const
{
	return Cast<UMovieScene>(GetOuter())->IsAMasterTrack(*this);
}

TInlineValue<FMovieSceneSegmentCompilerRules> UMovieSceneAtomTrack::GetRowCompilerRules() const
{
	struct FCompilerRules : FMovieSceneSegmentCompilerRules 
	{
		virtual void BlendSegment(FMovieSceneSegment& Segment, const TArrayView<const FMovieSceneSectionData>& SourceData) const
		{
			// Run the default high pass filter for overlap priority
			MovieSceneSegmentCompiler::BlendSegmentHighPass(Segment, SourceData);

			// Weed out based on array index (legacy behaviour)
			MovieSceneSegmentCompiler::BlendSegmentLegacySectionOrder(Segment, SourceData);
		}
	};
	return FCompilerRules();
}

/***************************************************************************
 *      関数定義
 *      Function Definition
 ***************************************************************************/

#undef LOCTEXT_NAMESPACE

/* --- end of file --- */
