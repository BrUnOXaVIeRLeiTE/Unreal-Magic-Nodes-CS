/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
///		Copyright 2021 (C) Bruno Xavier B. Leite
//////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "DiffTool_ListItem.h"

#include "KCS_NodeStyle.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define LOCTEXT_NAMESPACE "Synaptech"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

SDIFFListItem::SDIFFListItem() {}
SDIFFListItem::~SDIFFListItem(){}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SDIFFListItem::Construct(const FArguments &InArgs) {
	OnClicked = InArgs._OnClicked;
	//
	Line = InArgs._Line.Get();
	Owner = InArgs._Owner.Get();
	State = InArgs._State.Get();
	//
	DeletedStyle = &FEditorStyle::Get().GetWidgetStyle<FButtonStyle>("FlatButton.Danger");
	NewLineStyle = &FEditorStyle::Get().GetWidgetStyle<FButtonStyle>("FlatButton.Success");
	UnchangedStyle = &FEditorStyle::Get().GetWidgetStyle<FButtonStyle>("FlatButton.Info");
	PickedDeletedStyle = &FEditorStyle::Get().GetWidgetStyle<FButtonStyle>("FlatButton.Danger");
	PickedNewLineStyle = &FEditorStyle::Get().GetWidgetStyle<FButtonStyle>("FlatButton.Warning");
	//
	//
	if (Owner==EDIFF_Owner::Source) {
		SAssignNew(Button,SBorder)
		.VAlign(VAlign_Center).HAlign(HAlign_Fill).Padding(2)
		[
			SNew(STextBlock).Margin(FMargin(20,2,2,2))
			.Justification(ETextJustify::Right)
			.Text(this,&SDIFFListItem::GetCaption)
			.Font(FKCS_NodeStyle::Get().Get()->GetWidgetStyle<FTextBlockStyle>("KCS.CodeBlockStyle").Font)
		];
	}///
	//
	if (Owner==EDIFF_Owner::Target) {
		SAssignNew(Button,SBorder)
		.VAlign(VAlign_Center).HAlign(HAlign_Fill).Padding(2)
		[
			SNew(STextBlock).Margin(FMargin(2,2,20,2))
			.Justification(ETextJustify::Left)
			.Text(this,&SDIFFListItem::GetCaption)
			.Font(FKCS_NodeStyle::Get().Get()->GetWidgetStyle<FTextBlockStyle>("KCS.CodeBlockStyle").Font)
		];
	}///
	//
	SetButtonStyle();
	//
	//
	if (Owner==EDIFF_Owner::Source) {
		ChildSlot.HAlign(HAlign_Fill).VAlign(VAlign_Fill)
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			.VAlign(VAlign_Center).Padding(4,0,0,0)
			[
				Button.ToSharedRef()
			]
		];
	} else {
		ChildSlot.HAlign(HAlign_Fill).VAlign(VAlign_Fill)
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			.VAlign(VAlign_Center).Padding(0,0,4,0)
			[
				Button.ToSharedRef()
			]
		];
	}///
}

void SDIFFListItem::Tick(const FGeometry &Geometry, const double InCurrentTime, const float InDeltaTime) {
	SetButtonStyle();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool SDIFFListItem::IsInteractable() const {
	return IsEnabled();
}

bool SDIFFListItem::SupportsKeyboardFocus() const {
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SDIFFListItem::OnMouseEnter(const FGeometry &MyGeometry, const FPointerEvent &MouseEvent){}

void SDIFFListItem::OnMouseCaptureLost(const FCaptureLostEvent &CaptureLostEvent){}

void SDIFFListItem::OnMouseLeave(const FPointerEvent &MouseEvent) {
	SWidget::OnMouseLeave(MouseEvent);
}

FReply SDIFFListItem::OnMouseButtonDown(const FGeometry &MyGeometry, const FPointerEvent &MouseEvent) {
	FReply Reply = FReply::Unhandled();
	//
	if (OnClicked.IsBound()) {
		Reply = OnClicked.Execute();
	} else {
		switch (State) {
			case EDIFF_State::Deleted:
				State = EDIFF_State::PickedDeleted; break;
			case EDIFF_State::NewLine:
				State = EDIFF_State::PickedNewLine; break;
			case EDIFF_State::PickedDeleted:
				State = EDIFF_State::Deleted; break;
			case EDIFF_State::PickedNewLine:
				State = EDIFF_State::NewLine; break;
		default: break;}
	Reply=FReply::Handled();}
	//
	return Reply;
}

FReply SDIFFListItem::OnMouseButtonUp(const FGeometry &MyGeometry, const FPointerEvent &MouseEvent) {
	return FReply::Handled();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SDIFFListItem::SetLine(const FString &New) {
	Line = New;
}

void SDIFFListItem::SetOwner(const EDIFF_Owner InOwner) {
	Owner = InOwner;
}

void SDIFFListItem::SetState(const EDIFF_State InState) {
	State = InState;
}

void SDIFFListItem::SetButtonStyle() {
	if (!Button.IsValid()) {return;}
	//
	switch (State) {
		case EDIFF_State::Unchanged:
			Button->SetBorderImage(&UnchangedStyle->Normal); break;
		case EDIFF_State::Deleted:
			Button->SetBorderImage(&DeletedStyle->Normal); break;
		case EDIFF_State::NewLine:
			Button->SetBorderImage(&NewLineStyle->Normal); break;
		case EDIFF_State::PickedDeleted:
			Button->SetBorderImage(&PickedDeletedStyle->Pressed); break;
		case EDIFF_State::PickedNewLine:
			Button->SetBorderImage(&PickedNewLineStyle->Pressed); break;
	default: break;}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const EDIFF_Owner SDIFFListItem::GetOwner() const {
	return Owner;
}

const EDIFF_State SDIFFListItem::GetState() const {
	return State;
}

const FString & SDIFFListItem::GetLine() const {
	return Line;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

FText SDIFFListItem::GetCaption() const {
	return FText::FromString(Line);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#undef LOCTEXT_NAMESPACE

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////