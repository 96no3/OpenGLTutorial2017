/****************************************************************************
 *
 * CRI Middleware SDK
 *
 * Copyright (c) 2017 CRI Middleware Co., Ltd.
 *
 * Library  : CRIWARE plugin for Unreal Engine 4
 * Module   : Atom Track Editor
 * File     : AtomTrackEditor.h
 *
 ****************************************************************************/

/* 多重定義防止 */
#pragma once

/***************************************************************************
 *      インクルードファイル
 *      Include files
 ***************************************************************************/
/* Unreal Engine 4関連ヘッダ */
#include "ISequencer.h"
#include "AssetData.h"
#include "ISequencerTrackEditor.h"
#include "MovieSceneTrackEditor.h"
#include "SequencerSectionPainter.h"
#include "FloatCurveKeyArea.h"

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
/**
 * Tools for audio tracks
 */
class FAtomTrackEditor : public FMovieSceneTrackEditor
{
public:
	/**
	 * Constructor
	 *
	 * @param InSequencer The sequencer instance to be used by this tool
	 */
	FAtomTrackEditor(TSharedRef<ISequencer> InSequencer);

	/** Virtual destructor. */
	virtual ~FAtomTrackEditor();

	/**
	 * Creates an instance of this class.  Called by a sequencer
	 *
	 * @param OwningSequencer The sequencer instance to be used by this tool
	 * @return The new instance of this class
	 */
	static TSharedRef<ISequencerTrackEditor> CreateTrackEditor(TSharedRef<ISequencer> OwningSequencer);

public:
	// ISequencerTrackEditor interface
	virtual void BuildAddTrackMenu(FMenuBuilder& MenuBuilder) override;
	virtual TSharedPtr<SWidget> BuildOutlinerEditWidget(const FGuid& ObjectBinding, UMovieSceneTrack* Track, const FBuildEditWidgetParams& Params) override;
	virtual bool HandleAssetAdded(UObject* Asset, const FGuid& TargetObjectGuid) override;
	virtual TSharedRef<ISequencerSection> MakeSectionInterface(UMovieSceneSection& SectionObject, UMovieSceneTrack& Track, FGuid ObjectBinding) override;
	virtual bool SupportsType(TSubclassOf<UMovieSceneTrack> Type) const override;
	virtual void BuildTrackContextMenu( FMenuBuilder& MenuBuilder, UMovieSceneTrack* Track ) override;
	virtual const FSlateBrush* GetIconBrush() const override;

protected:
	/** Delegate for AnimatablePropertyChanged in HandleAssetAdded for master sounds */
	FKeyPropertyResult AddNewMasterSound(float KeyTime, class USoundAtomCue* Sound);

	/** Delegate for AnimatablePropertyChanged in HandleAssetAdded for attached sounds */
	FKeyPropertyResult AddNewAttachedSound(float KeyTime, class USoundAtomCue* Sound, TArray<TWeakObjectPtr<UObject>> ObjectsToAttachTo);

private:
	/** Callback for executing the "Add Event Track" menu entry. */
	void HandleAddAtomTrackMenuEntryExecute();

	/** Atom sub menu */
	TSharedRef<SWidget> BuildAtomSubMenu(UMovieSceneTrack* Track);

	/** Atom asset selected */
	void OnAtomAssetSelected(const FAssetData& AssetData, UMovieSceneTrack* Track);
};

class FAtomSection : public ISequencerSection, public TSharedFromThis<FAtomSection>
{
public:
	/** Constructor. */
	FAtomSection(UMovieSceneSection& InSection, bool bOnAMasterTrack, TWeakPtr<ISequencer> InSequencer);

	/** Virtual destructor. */
	virtual ~FAtomSection();

public:
	// ISequencerSection interface
	virtual UMovieSceneSection* GetSectionObject() override;
	virtual FText GetSectionTitle() const override;
	virtual float GetSectionHeight() const override;
	virtual void GenerateSectionLayout( class ISectionLayoutBuilder& LayoutBuilder) const override;
	virtual int32 OnPaintSection(FSequencerSectionPainter& Painter) const override;
	virtual void Tick(const FGeometry& AllottedGeometry, const FGeometry& ParentGeometry, const double InCurrentTime, const float InDeltaTime) override;

private:
	/** The section we are visualizing. */
	UMovieSceneSection& Section;

	/** Sound volume/pitch key areas. */
	mutable TSharedPtr<FFloatCurveKeyArea> SoundVolumeArea;
	mutable TSharedPtr<FFloatCurveKeyArea> PitchMultiplierArea;

	/** Stored data about the waveform to determine when it is invalidated. */
	TRange<float> StoredDrawRange;
	bool StoredShowIntensity;
	float StoredStartOffset;
	int32 StoredXOffset;
	int32 StoredXSize;
	FColor StoredColor;

	/** Stored sound wave to determine when it is invalidated. */
	TWeakObjectPtr<USoundAtomCue> StoredSoundAtomCue;

	/** Whether this section is on a master audio track or an attached audio track. */
	bool bIsOnAMasterTrack;

	TWeakPtr<ISequencer> Sequencer;
};

/***************************************************************************
 *      関数宣言
 *      Prototype Functions
 ***************************************************************************/

/* --- end of file --- */
