/****************************************************************************
 *
 * CRI Middleware SDK
 *
 * Copyright (c) 2016-2017 CRI Middleware Co., Ltd.
 *
 * Library  : CRIWARE plugin for Unreal Engine 4
 * Module   : Atom ASR Rack
 * File     : AtomAsrRack.cpp
 *
 ****************************************************************************/

/***************************************************************************
 *      インクルードファイル
 *      Include files
 ***************************************************************************/
/* モジュールヘッダ */
#include "AtomTrackEditor.h"

/* CRIWAREプラグインヘッダ */
#include "CriWareSequencerPrivatePCH.h"
#include "Tracks/MovieSceneAtomTrack.h"
#include "Sections/MovieSceneAtomSection.h"

/* Unreal Engine 4関連ヘッダ */
#include "SlateApplication.h"
#include "SBoxPanel.h"
#include "SBox.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "EditorStyleSet.h"
#include "SequencerUtilities.h"
#include "AssetRegistryModule.h"
#include "IContentBrowserSingleton.h"
#include "ContentBrowserModule.h"
#include "ISectionLayoutBuilder.h"

/***************************************************************************
 *      定数マクロ
 *      Macro Constants
 ***************************************************************************/
#define LOCTEXT_NAMESPACE "FAtomTrackEditor"

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
FAtomTrackEditor::FAtomTrackEditor(TSharedRef<ISequencer> InSequencer)
	: FMovieSceneTrackEditor(InSequencer)
{
}

FAtomTrackEditor::~FAtomTrackEditor()
{
}

TSharedRef<ISequencerTrackEditor> FAtomTrackEditor::CreateTrackEditor(TSharedRef<ISequencer> InSequencer)
{
	return MakeShareable(new FAtomTrackEditor(InSequencer));
}

void FAtomTrackEditor::BuildAddTrackMenu(FMenuBuilder& MenuBuilder)
{
	MenuBuilder.AddMenuEntry(
		LOCTEXT("AddTrack", "Atom Track"),
		LOCTEXT("AddTooltip", "Adds a new master audio track that can play sounds."),
		FSlateIcon(FEditorStyle::GetStyleSetName(), "Sequencer.Tracks.Atom"),
		FUIAction(
			FExecuteAction::CreateRaw(this, &FAtomTrackEditor::HandleAddAtomTrackMenuEntryExecute)
		)
	);
}

bool FAtomTrackEditor::SupportsType(TSubclassOf<UMovieSceneTrack> Type) const
{
	return Type == UMovieSceneAtomTrack::StaticClass();
}

void FAtomTrackEditor::BuildTrackContextMenu(FMenuBuilder& MenuBuilder, UMovieSceneTrack* Track)
{
}

const FSlateBrush* FAtomTrackEditor::GetIconBrush() const
{
	return FEditorStyle::GetBrush("Sequencer.Tracks.Audio");
}

TSharedRef<ISequencerSection> FAtomTrackEditor::MakeSectionInterface(UMovieSceneSection& SectionObject, UMovieSceneTrack& Track, FGuid ObjectBinding)
{
	check(SupportsType(SectionObject.GetOuter()->GetClass()));

	bool bIsAMasterTrack = Cast<UMovieScene>(Track.GetOuter())->IsAMasterTrack(Track);
	return MakeShareable(new FAtomSection(SectionObject, bIsAMasterTrack, GetSequencer()));
}

TSharedPtr<SWidget> FAtomTrackEditor::BuildOutlinerEditWidget(const FGuid& ObjectBinding, UMovieSceneTrack* Track, const FBuildEditWidgetParams& Params)
{
	// Create a container edit box
	return SNew(SHorizontalBox)

	// Add the audio combo box
	+ SHorizontalBox::Slot()
	.AutoWidth()
	.VAlign(VAlign_Center)
	[
		FSequencerUtilities::MakeAddButton(LOCTEXT("AtomText", "Atom"), FOnGetContent::CreateSP(this, &FAtomTrackEditor::BuildAtomSubMenu, Track), Params.NodeIsHovered)
	];
}

bool FAtomTrackEditor::HandleAssetAdded(UObject* Asset, const FGuid& TargetObjectGuid)
{
	if (Asset->IsA<USoundAtomCue>()) {
		auto Sound = Cast<USoundAtomCue>(Asset);

		if (TargetObjectGuid.IsValid()) {
			TArray<TWeakObjectPtr<>> OutObjects;
			for (TWeakObjectPtr<> Object : GetSequencer()->FindObjectsInCurrentSequence(TargetObjectGuid)) {
				OutObjects.Add(Object);
			}

			AnimatablePropertyChanged(FOnKeyProperty::CreateRaw(this, &FAtomTrackEditor::AddNewAttachedSound, Sound, OutObjects));
		} else {
			AnimatablePropertyChanged(FOnKeyProperty::CreateRaw(this, &FAtomTrackEditor::AddNewMasterSound, Sound));
		}

		return true;
	}
	return false;
}

FKeyPropertyResult FAtomTrackEditor::AddNewMasterSound(float KeyTime, USoundAtomCue* Sound)
{
	FKeyPropertyResult KeyPropertyResult;

	FFindOrCreateMasterTrackResult<UMovieSceneAtomTrack> TrackResult = FindOrCreateMasterTrack<UMovieSceneAtomTrack>();
	UMovieSceneTrack* Track = TrackResult.Track;

	auto AtomTrack = Cast<UMovieSceneAtomTrack>(Track);
	AtomTrack->AddNewSound(Sound, KeyTime);

	TArray<UMovieSceneSection*> AllSections = AtomTrack->GetAllSections();
	if (AllSections.Num() > 0) {
		for (auto MovieSceneSection : AllSections) {
			float EndTime = MovieSceneSection->GetEndTime();
			if (EndTime > (MAX_int32 / 1000.0f)) {
				MovieSceneSection->SetIsInfinite(true);
			}
		}
	}

	if (TrackResult.bWasCreated) {
		AtomTrack->SetDisplayName(LOCTEXT("AtomTrackName", "Atom"));
	}

	KeyPropertyResult.bTrackModified = true;

	return KeyPropertyResult;
}

FKeyPropertyResult FAtomTrackEditor::AddNewAttachedSound(float KeyTime, USoundAtomCue* Sound, TArray<TWeakObjectPtr<UObject>> ObjectsToAttachTo)
{
	FKeyPropertyResult KeyPropertyResult;

	for(int32 ObjectIndex = 0; ObjectIndex < ObjectsToAttachTo.Num(); ++ObjectIndex) {
		UObject* Object = ObjectsToAttachTo[ObjectIndex].Get();

		FFindOrCreateHandleResult HandleResult = FindOrCreateHandleToObject(Object);
		FGuid ObjectHandle = HandleResult.Handle;
		KeyPropertyResult.bHandleCreated |= HandleResult.bWasCreated;

		if (ObjectHandle.IsValid()) {
			FFindOrCreateTrackResult TrackResult = FindOrCreateTrackForObject(ObjectHandle, UMovieSceneAtomTrack::StaticClass());
			UMovieSceneTrack* Track = TrackResult.Track;
			KeyPropertyResult.bTrackCreated |= TrackResult.bWasCreated;

			if (ensure(Track)) {
				auto AtomTrack = Cast<UMovieSceneAtomTrack>(Track);
				AtomTrack->AddNewSound(Sound, KeyTime);
				AtomTrack->SetDisplayName(LOCTEXT("AtomTrackName", "Atom"));
				KeyPropertyResult.bTrackModified = true;
			}
		}
	}

	return KeyPropertyResult;
}

void FAtomTrackEditor::HandleAddAtomTrackMenuEntryExecute()
{
	UMovieScene* FocusedMovieScene = GetFocusedMovieScene();

	if (FocusedMovieScene == nullptr) {
		return;
	}

	const FScopedTransaction Transaction(NSLOCTEXT("Sequencer", "AddAtomTrack_Transaction", "Add Atom Track"));
	FocusedMovieScene->Modify();

	auto NewTrack = FocusedMovieScene->AddMasterTrack<UMovieSceneAtomTrack>();
	ensure(NewTrack);
	NewTrack->SetDisplayName(LOCTEXT("AtomTrackName", "Atom"));
	GetSequencer()->NotifyMovieSceneDataChanged(EMovieSceneDataChangeType::MovieSceneStructureItemAdded);
}

TSharedRef<SWidget> FAtomTrackEditor::BuildAtomSubMenu(UMovieSceneTrack* Track)
{
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	TArray<FName> ClassNames;
	ClassNames.Add(USoundAtomCue::StaticClass()->GetFName());
	TSet<FName> DerivedClassNames;
	AssetRegistryModule.Get().GetDerivedClassNames(ClassNames, TSet<FName>(), DerivedClassNames);

	FMenuBuilder MenuBuilder(true, nullptr);

	FAssetPickerConfig AssetPickerConfig;
	{
		AssetPickerConfig.OnAssetSelected = FOnAssetSelected::CreateRaw(this, &FAtomTrackEditor::OnAtomAssetSelected, Track);
		AssetPickerConfig.bAllowNullSelection = false;
		AssetPickerConfig.InitialAssetViewType = EAssetViewType::List;
		for (auto ClassName : DerivedClassNames) {
			AssetPickerConfig.Filter.ClassNames.Add(ClassName);
		}
	}

	FContentBrowserModule& ContentBrowserModule = FModuleManager::Get().LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));

	TSharedPtr<SBox> MenuEntry = SNew(SBox)
		.WidthOverride(300.0f)
		.HeightOverride(300.f)
		[
			ContentBrowserModule.Get().CreateAssetPicker(AssetPickerConfig)
		];

	MenuBuilder.AddWidget(MenuEntry.ToSharedRef(), FText::GetEmpty(), true);

	return MenuBuilder.MakeWidget();
}

void FAtomTrackEditor::OnAtomAssetSelected(const FAssetData& AssetData, UMovieSceneTrack* Track)
{
	FSlateApplication::Get().DismissAllMenus();

	UObject* SelectedObject = AssetData.GetAsset();

	if (SelectedObject) {
		USoundAtomCue* NewSound = CastChecked<USoundAtomCue>(AssetData.GetAsset());
		if (NewSound != nullptr) {
			const FScopedTransaction Transaction(NSLOCTEXT("Sequencer", "AddAtom_Transaction", "Add Atom"));

			auto AtomTrack = Cast<UMovieSceneAtomTrack>(Track);
			AtomTrack->Modify();

			float KeyTime = GetSequencer()->GetLocalTime();
			AtomTrack->AddNewSound(NewSound, KeyTime);

			GetSequencer()->NotifyMovieSceneDataChanged(EMovieSceneDataChangeType::MovieSceneStructureItemAdded);
		}
	}
}

FAtomSection::FAtomSection(UMovieSceneSection& InSection, bool bOnAMasterTrack, TWeakPtr<ISequencer> InSequencer)
	: Section(InSection)
	, StoredDrawRange(TRange<float>::Empty())
	, bIsOnAMasterTrack(bOnAMasterTrack)
	, Sequencer(InSequencer)
{
}

FAtomSection::~FAtomSection()
{
}

UMovieSceneSection* FAtomSection::GetSectionObject()
{
	return &Section;
}

FText FAtomSection::GetSectionTitle() const
{
	UMovieSceneAtomSection* AtomSection = Cast<UMovieSceneAtomSection>(&Section);
	if (AtomSection && AtomSection->GetSound()) {
		return FText::FromString(AtomSection->GetSound()->GetName());
	}

	return NSLOCTEXT("FAtomSection", "NoAtomTitleName", "No Atom");
}

float FAtomSection::GetSectionHeight() const
{
	return Section.GetTypedOuter<UMovieSceneAtomTrack>()->GetRowHeight();
}

void FAtomSection::GenerateSectionLayout(class ISectionLayoutBuilder& LayoutBuilder) const
{
	UMovieSceneAtomSection* AtomSection = Cast<UMovieSceneAtomSection>(&Section);

	SoundVolumeArea = MakeShareable(new FFloatCurveKeyArea(&AtomSection->GetSoundVolumeCurve(), AtomSection)) ;
	PitchMultiplierArea = MakeShareable(new FFloatCurveKeyArea(&AtomSection->GetPitchMultiplierCurve(), AtomSection));

	LayoutBuilder.AddKeyArea("Volume", NSLOCTEXT("FAtomSection", "SoundVolumeArea", "Volume"), SoundVolumeArea.ToSharedRef());
	LayoutBuilder.AddKeyArea("Pitch Multiplier", NSLOCTEXT("FAtomSection", "PitchMultiplierArea", "Pitch Multiplier"), PitchMultiplierArea.ToSharedRef());
}

int32 FAtomSection::OnPaintSection(FSequencerSectionPainter& Painter) const
{
	int32 LayerId = Painter.PaintSectionBackground();

	return LayerId;
}

void FAtomSection::Tick(const FGeometry& AllottedGeometry, const FGeometry& ParentGeometry, const double InCurrentTime, const float InDeltaTime)
{
}

/***************************************************************************
 *      関数定義
 *      Function Definition
 ***************************************************************************/

#undef LOCTEXT_NAMESPACE

/* --- end of file --- */
