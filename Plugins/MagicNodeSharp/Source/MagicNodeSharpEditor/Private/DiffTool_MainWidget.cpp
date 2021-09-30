/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
///		Copyright 2021 (C) Bruno Xavier B. Leite
//////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "DiffTool_MainWidget.h"

#include "MagicNodeSharp.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define LOCTEXT_NAMESPACE "Synaptech"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EDIFF_Mod SDIFFMainWidget::Modifier = EDIFF_Mod::Normal;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SDIFFMainWidget::Construct(const FArguments &InArgs) {
	SAssignNew(DIFF_SOURCE_LIST,SScrollBox).OnUserScrolled(this,&SDIFFMainWidget::OnSourceScrolled);
	SAssignNew(DIFF_TARGET_LIST,SScrollBox).OnUserScrolled(this,&SDIFFMainWidget::OnTargetScrolled);
	SAssignNew(DIFF_PANEL,SBox).HAlign(HAlign_Fill).VAlign(VAlign_Fill);
	//
	//
	SAssignNew(DIFF_PANEL,SBox)
	.HAlign(HAlign_Fill).VAlign(VAlign_Fill)
	[
		SNew(SVerticalBox)
		+SVerticalBox::Slot().Padding(0,2,0,2)
		.VAlign(VAlign_Fill).HAlign(HAlign_Fill)
		[
			SNew(SSplitter)
			.Orientation(EOrientation::Orient_Horizontal)
			+SSplitter::Slot().Value(0.5f)
			[
				SNew(SBorder).VAlign(VAlign_Fill).HAlign(HAlign_Fill)
				.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
				[
					SNew(SVerticalBox)
					+SVerticalBox::Slot().Padding(0,2,0,2)
					.VAlign(VAlign_Fill).HAlign(HAlign_Fill)
					[
						DIFF_SOURCE_LIST.ToSharedRef()
					]
				]
			]
			+SSplitter::Slot().Value(0.5f)
			[
				SNew(SBorder).VAlign(VAlign_Fill).HAlign(HAlign_Fill)
				.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
				[
					SNew(SVerticalBox)
					+SVerticalBox::Slot().Padding(0,2,0,2)
					.VAlign(VAlign_Fill).HAlign(HAlign_Fill)
					[
						DIFF_TARGET_LIST.ToSharedRef()
					]
				]
			]
		]
		+SVerticalBox::Slot()
		.AutoHeight().Padding(0,2,0,0)
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			[
				SNew(SBox)
				.VAlign(VAlign_Fill).HAlign(HAlign_Fill)
				.MinDesiredWidth(100.f).MinDesiredHeight(44.f)
				[
					SAssignNew(DIFF_Button_APPLY,SButton)
					.ButtonStyle(FEditorStyle::Get(),"FlatButton.Default")
					.IsEnabled(this,&SDIFFMainWidget::IsScriptCompare)
					.OnClicked(this,&SDIFFMainWidget::OnClickedApply)
					.ForegroundColor(FSlateColor::UseForeground())
					.VAlign(VAlign_Center).HAlign(HAlign_Fill)
					[
						SNew(STextBlock)
						.Justification(ETextJustify::Center)
						.Text(LOCTEXT("DIFF_Button_APPLY","APPLY"))
						.Font(FEditorStyle::GetFontStyle(TEXT("BoldFont")))
						
					]
				]
			]
		]
	];///
	//
	ChildSlot.HAlign(HAlign_Fill).VAlign(VAlign_Fill)
	[
		DIFF_PANEL.ToSharedRef()
	];//
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SDIFFMainWidget::RebuildListViews(UMagicNodeSharpSource* Source, UMagicNodeSharpSource* Target) {
	if (Source==nullptr||Target==nullptr) {return;}
	//
	SourceObject = Source;
	TargetObject = Target;
	//
	RebuildSourceListView();
	RebuildTargetListView();
}

void SDIFFMainWidget::RebuildSourceListView() {
	if (!SourceObject.IsValid()||!TargetObject.IsValid()) {return;}
	//
	DIFF_SOURCE_ITEMS.Empty();
	DIFF_SOURCE_LIST->ClearChildren();
	//
	TArray<FString>SourceInfo;
	TArray<FString>TargetInfo;
	//
	SourceObject->Source.ParseIntoArrayLines(SourceInfo,false);
	TargetObject->Source.ParseIntoArrayLines(TargetInfo,false);
	//
	int32 Bulk = (SourceInfo.Num()>=TargetInfo.Num()) ? SourceInfo.Num() : TargetInfo.Num();
	for (int32 I=0; I<Bulk; I++){
		DIFF_SOURCE_ITEMS.Add(TSharedPtr<SDIFFListItem>(nullptr));
		//
		if (!SourceInfo.IsValidIndex(I)){SourceInfo.Add(FString());}
		else if (!TargetInfo.IsValidIndex(I)){TargetInfo.Add(FString());}
	}///
	//
	for (int32 I=0; I<DIFF_SOURCE_ITEMS.Num(); I++) {
		EDIFF_State State = EDIFF_State::Unchanged;
		const FString &Line = SourceInfo[I];
		const FName Name = *SourceInfo[I];
		//
		if (Line.Compare(TargetInfo[I])==0) {
			State = EDIFF_State::Unchanged;
		} else {State=EDIFF_State::NewLine;}
		//
		SAssignNew(DIFF_SOURCE_ITEMS[I],SDIFFListItem)
		.OnClicked(this,&SDIFFMainWidget::OnClickedSourceListItem,Name)
		.Owner(EDIFF_Owner::Source).State(State).Line(Line);
	}///
	//
	for (int32 I=0; I<DIFF_SOURCE_ITEMS.Num(); I++) {
		DIFF_SOURCE_LIST->AddSlot().VAlign(VAlign_Top).HAlign(HAlign_Fill)
		[
			DIFF_SOURCE_ITEMS[I].ToSharedRef()
		];
	}///
	//
	DIFF_SOURCE_LIST->Invalidate(EInvalidateWidget::Layout);
}

void SDIFFMainWidget::RebuildTargetListView() {
	if (!SourceObject.IsValid()||!TargetObject.IsValid()) {return;}
	//
	DIFF_TARGET_ITEMS.Empty();
	DIFF_TARGET_LIST->ClearChildren();
	//
	TArray<FString>TargetInfo;
	TArray<FString>SourceInfo;
	//
	SourceObject->Source.ParseIntoArrayLines(SourceInfo,false);
	TargetObject->Source.ParseIntoArrayLines(TargetInfo,false);
	//
	//
	int32 Bulk = (TargetInfo.Num()>=SourceInfo.Num()) ? TargetInfo.Num() : SourceInfo.Num();
	for (int32 I=0; I<Bulk; I++){
		DIFF_TARGET_ITEMS.Add(TSharedPtr<SDIFFListItem>(nullptr));
		//
		if (!TargetInfo.IsValidIndex(I)){TargetInfo.Add(FString());}
		else if (!SourceInfo.IsValidIndex(I)){SourceInfo.Add(FString());}
	}///
	//
	for (int32 I=0; I<DIFF_TARGET_ITEMS.Num(); I++) {
		EDIFF_State State = EDIFF_State::Unchanged;
		const FString &Line = TargetInfo[I];
		const FName Name = *TargetInfo[I];
		//
		if (SourceInfo.IsValidIndex(I)&&(Line.Compare(SourceInfo[I])==0)){
			State = EDIFF_State::Unchanged;
		} else {State=EDIFF_State::Deleted;}
		//
		SAssignNew(DIFF_TARGET_ITEMS[I],SDIFFListItem)
		.OnClicked(this,&SDIFFMainWidget::OnClickedTargetListItem,Name)
		.Owner(EDIFF_Owner::Target).State(State).Line(Line);
	}///
	//
	for (int32 I=0; I<DIFF_TARGET_ITEMS.Num(); I++) {
		DIFF_TARGET_LIST->AddSlot().VAlign(VAlign_Top).HAlign(HAlign_Fill)
		[
			DIFF_TARGET_ITEMS[I].ToSharedRef()
		];
	}///
	//
	DIFF_TARGET_LIST->Invalidate(EInvalidateWidget::Layout);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

FText SDIFFMainWidget::GetSourceName() const {
	if (!SourceObject.IsValid()) {
		return FText::FromString(TEXT("(Script)"));
	}///
	//
	FString SN = SourceObject->GetName().Replace(TEXT("Default__"),TEXT("(Script)  "));
	SN.RemoveFromEnd(TEXT("_C")); SN.RemoveFromStart(TEXT("GEN_"));
	//
	return FText::FromString(SN+CS_SCRIPT_EXT);
}

FText SDIFFMainWidget::GetTargetName() const {
	if (!TargetObject.IsValid()) {
		return FText::FromString(TEXT("(Script)"));
	}///
	//
	FString SN = TargetObject->GetName().Replace(TEXT("Default__"),TEXT("(Script)  "));
	SN.RemoveFromEnd(TEXT("_C")); SN.RemoveFromStart(TEXT("GEN_"));
	//
	return FText::FromString(SN+CS_SCRIPT_EXT);
}

int32 SDIFFMainWidget::FindFirstPickedSource() const {
	for (int32 I=0; I<DIFF_SOURCE_ITEMS.Num(); I++) {
		if (!DIFF_SOURCE_ITEMS[I].IsValid()) {continue;}
		if (DIFF_SOURCE_ITEMS[I]->GetState()==EDIFF_State::PickedNewLine||DIFF_SOURCE_ITEMS[I]->GetState()==EDIFF_State::PickedDeleted) {return I;}
	}///
	//
	return INDEX_NONE;
}

int32 SDIFFMainWidget::FindFirstPickedTarget() const {
	for (int32 I=0; I<DIFF_TARGET_ITEMS.Num(); I++) {
		if (!DIFF_TARGET_ITEMS[I].IsValid()) {continue;}
		if (DIFF_TARGET_ITEMS[I]->GetState()==EDIFF_State::PickedNewLine||DIFF_TARGET_ITEMS[I]->GetState()==EDIFF_State::PickedDeleted) {return I;}
	}///
	//
	return INDEX_NONE;
}

int32 SDIFFMainWidget::FindLastPickedSource() const {
	for (int32 I=DIFF_SOURCE_ITEMS.Num()-1; I>=0; I--) {
		if (!DIFF_SOURCE_ITEMS[I].IsValid()) {continue;}
		if (DIFF_SOURCE_ITEMS[I]->GetState()==EDIFF_State::PickedNewLine||DIFF_SOURCE_ITEMS[I]->GetState()==EDIFF_State::PickedDeleted) {return I;}
	}///
	//
	return INDEX_NONE;
}

int32 SDIFFMainWidget::FindLastPickedTarget() const {
	for (int32 I=DIFF_TARGET_ITEMS.Num()-1; I>=0; I--) {
		if (!DIFF_TARGET_ITEMS[I].IsValid()) {continue;}
		if (DIFF_TARGET_ITEMS[I]->GetState()==EDIFF_State::PickedNewLine||DIFF_TARGET_ITEMS[I]->GetState()==EDIFF_State::PickedDeleted) {return I;}
	}///
	//
	return INDEX_NONE;
}

TSharedPtr<SDIFFListItem> SDIFFMainWidget::FindSourceWidget(const FString &Line) {
	for (int32 I=0; I<DIFF_SOURCE_ITEMS.Num(); I++) {
		if (!DIFF_SOURCE_ITEMS[I].IsValid()) {continue;}
		if (DIFF_TARGET_ITEMS[I]->GetLine().Compare(Line)==0) {return DIFF_SOURCE_ITEMS[I];}
	}///
	//
	return TSharedPtr<SDIFFListItem>(nullptr);
}

TSharedPtr<SDIFFListItem> SDIFFMainWidget::FindTargetWidget(const FString &Line) {
	for (int32 I=0; I<DIFF_TARGET_ITEMS.Num(); I++) {
		if (!DIFF_TARGET_ITEMS[I].IsValid()) {continue;}
		if (DIFF_TARGET_ITEMS[I]->GetLine().Compare(Line)==0) {return DIFF_TARGET_ITEMS[I];}
	}///
	//
	return TSharedPtr<SDIFFListItem>(nullptr);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SDIFFMainWidget::OnSourceScrolled(float Offset) {DIFF_TARGET_LIST->SetScrollOffset(Offset);}
void SDIFFMainWidget::OnTargetScrolled(float Offset) {DIFF_SOURCE_LIST->SetScrollOffset(Offset);}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

FReply SDIFFMainWidget::OnClickedSourceListItem(const FName Item) {
	auto Widget = FindSourceWidget(Item.ToString());
	//
	if (!IsEnabled()) {return FReply::Handled();}
	if (!Widget.IsValid()) {return FReply::Handled();}
	//
	const auto State = Widget->GetState();
	//
	switch (State) {
		case EDIFF_State::Deleted:
			Widget->SetState(EDIFF_State::PickedDeleted); break;
		case EDIFF_State::PickedDeleted:
			Widget->SetState(EDIFF_State::Deleted); break;
		case EDIFF_State::NewLine:
			Widget->SetState(EDIFF_State::PickedNewLine); break;
		case EDIFF_State::PickedNewLine:
			Widget->SetState(EDIFF_State::NewLine); break;
	default: break;}
	//
	return FReply::Handled();
}

FReply SDIFFMainWidget::OnClickedTargetListItem(const FName Item) {
	auto Widget = FindTargetWidget(Item.ToString());
	//
	if (!IsEnabled()) {return FReply::Handled();}
	if (!Widget.IsValid()) {return FReply::Handled();}
	//
	const auto State = Widget->GetState();
	//
	switch (State) {
		case EDIFF_State::Deleted:
			Widget->SetState(EDIFF_State::PickedDeleted); break;
		case EDIFF_State::PickedDeleted:
			Widget->SetState(EDIFF_State::Deleted); break;
		case EDIFF_State::NewLine:
			Widget->SetState(EDIFF_State::PickedNewLine); break;
		case EDIFF_State::PickedNewLine:
			Widget->SetState(EDIFF_State::NewLine); break;
	default: break;}
	//
	return FReply::Handled();
}

FReply SDIFFMainWidget::OnClickedApply() {
	if (!IsEnabled()) {return FReply::Handled();}
	//
	TargetObject->Modify(true);
	TargetObject->Source = SourceObject->Source;
	//
	RebuildSourceListView();
	RebuildTargetListView();
	//
	return FReply::Handled();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

FReply SDIFFMainWidget::OnKeyDown(const FGeometry &MyGeometry, const FKeyEvent &InKeyEvent) {
	if (!IsEnabled()) {return FReply::Handled();}
	//
	if (InKeyEvent.GetKey()==EKeys::LeftControl||InKeyEvent.GetKey()==EKeys::RightControl) {SDIFFMainWidget::Modifier=EDIFF_Mod::Ctrl;}
	if (InKeyEvent.GetKey()==EKeys::LeftShift||InKeyEvent.GetKey()==EKeys::RightShift) {SDIFFMainWidget::Modifier=EDIFF_Mod::Shift;}
	if (InKeyEvent.GetKey()==EKeys::LeftAlt||InKeyEvent.GetKey()==EKeys::RightAlt) {SDIFFMainWidget::Modifier=EDIFF_Mod::Alt;}
	//
	return FReply::Handled();
}

FReply SDIFFMainWidget::OnKeyUp(const FGeometry &MyGeometry, const FKeyEvent &InKeyEvent) {
	if (!IsEnabled()) {return FReply::Handled();}
	//
	if (InKeyEvent.GetKey()==EKeys::LeftControl||InKeyEvent.GetKey()==EKeys::RightControl) {SDIFFMainWidget::Modifier=EDIFF_Mod::Normal;}
	if (InKeyEvent.GetKey()==EKeys::LeftShift||InKeyEvent.GetKey()==EKeys::RightShift) {SDIFFMainWidget::Modifier=EDIFF_Mod::Normal;}
	if (InKeyEvent.GetKey()==EKeys::LeftAlt||InKeyEvent.GetKey()==EKeys::RightAlt) {SDIFFMainWidget::Modifier=EDIFF_Mod::Normal;}
	//
	return FReply::Handled();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool SDIFFMainWidget::IsScriptCompare() const {
	if (!SourceObject.IsValid()||!TargetObject.IsValid()) {return false;}
	//
	return !(TargetObject->Source.Compare(SourceObject->Source)==0);
}

bool SDIFFMainWidget::IsInteractable() const {
	return IsEnabled();
}

bool SDIFFMainWidget::SupportsKeyboardFocus() const {
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#undef LOCTEXT_NAMESPACE

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////