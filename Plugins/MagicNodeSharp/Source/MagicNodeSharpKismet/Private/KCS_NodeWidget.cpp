//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
///		Copyright 2021 (C) Bruno Xavier B. Leite
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "KCS_NodeWidget.h"

#include "KCS_NodeStyle.h"
#include "KCS_TextSyntaxHighlighter.h"

#include "MCS_API.h"

#include "Runtime/Core/Public/Misc/CString.h"
#include "Runtime/InputCore/Classes/InputCoreTypes.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define LOCTEXT_NAMESPACE "Synaptech"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// CS Main Widget Constructors:

SKCS_MagicNodeWidget::SKCS_MagicNodeWidget(){}
SKCS_MagicNodeWidget::~SKCS_MagicNodeWidget(){}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// CS Main Widget:

void SKCS_MagicNodeWidget::Construct(const FArguments &InArgs, UKCS_MagicNode* InGraphNode) {
	GraphNode = InGraphNode; if (GraphNode) {UpdateGraphNode();}
}

void SKCS_MagicNodeWidget::Tick(const FGeometry &Geometry, const double CurrentTime, const float DeltaTime) {
	HintTimer = (HintTimer>=16.f) ? 0.f : (HintTimer+1.f);
	//
	if (IsCompiling()) {
		if (CompilerProgress >= 1.f) {
			CompilerProgress = 0.f;
		} else {CompilerProgress+=0.05f;}
	} else if (CompilerProgress > 0.f) {
		CompilerProgress = 0.f;
	}///
	//
	SGraphNode::Tick(Geometry,CurrentTime,DeltaTime);
}

void SKCS_MagicNodeWidget::CreateBelowWidgetControls(TSharedPtr<SVerticalBox>MainBox) {
	UMagicNodeSharpSource* Script = (CastChecked<UKCS_MagicNode>(GraphNode))->GetScriptSource();
	//
	MARSHALL = FTextSyntaxHighlighter::Create(
		FTextSyntaxHighlighter::FSyntaxTextStyle()
	);///
	//
	if (MARSHALL.IsValid()) {MARSHALL->SetScriptSource(Script);}
	//
	ViewSearchBox = false;
	ErrorLine = INDEX_NONE;
	SetLineCountList(GetLineCount());
	//
	//
	VS_SCROLL = SNew(SScrollBar)
	.OnUserScrolled(this,&SKCS_MagicNodeWidget::OnInternalVerticalScroll)
	.Thickness(FVector2D(8.f,8.f)).AlwaysShowScrollbar(false)
	.Orientation(Orient_Vertical);
	//
	HSS_SCROLL = SNew(SScrollBar)
	.OnUserScrolled(this,&SKCS_MagicNodeWidget::OnScriptHorizontalScroll)
	.Orientation(Orient_Horizontal).AlwaysShowScrollbar(false)
	.Thickness(FVector2D(8.f,8.f));
	//
	//
	MainBox->AddSlot()
	.VAlign(VAlign_Top).HAlign(HAlign_Fill)
	[
		SNew(SHorizontalBox)
		+SHorizontalBox::Slot()
		.VAlign(VAlign_Fill).HAlign(HAlign_Fill)
		[
			SNew(SBorder).Padding(1)
			.Visibility(this,&SKCS_MagicNodeWidget::GetScriptEditorVisibility)
			.BorderImage(FEditorStyle::GetBrush("Graph.Node.Body"))
			.ColorAndOpacity(FLinearColor(1.f,1.f,1.f,0.8f))
			[
				SNew(SVerticalBox)
				+SVerticalBox::Slot()
				.AutoHeight().Padding(10,5,10,5)
				[
					SNew(SHorizontalBox)
					+SHorizontalBox::Slot().HAlign(HAlign_Fill)
					[
						SAssignNew(SCRIPT_PICKER,SContentReference)
						.AssetReference(this,&SKCS_MagicNodeWidget::GetScriptSource)
						.OnSetReference(this,&SKCS_MagicNodeWidget::SetScriptSource)
						.AllowedClass(UMagicNodeSharpSource::StaticClass())
						.AssetPickerSizeOverride(FVector2D(540.f,360.f))
						.ShowFindInBrowserButton(true)
						.AllowClearingReference(true)
						.AllowSelectingNewAsset(true)
						.ShowToolsButton(false)
						.WidthOverride(400.f)
					]
				]
				+SVerticalBox::Slot()
				.FillHeight(1.f).Padding(10,5,10,5)
				[
					SNew(SHorizontalBox)
					+SHorizontalBox::Slot()
					.VAlign(VAlign_Fill).HAlign(HAlign_Fill)
					[
						SNew(SOverlay)
						+SOverlay::Slot()
						.VAlign(VAlign_Fill).HAlign(HAlign_Fill)
						[
							SNew(SBox)
							.VAlign(VAlign_Fill).HAlign(HAlign_Fill)
							.MinDesiredWidth(400.f).MinDesiredHeight(250.f)
							.MaxDesiredHeight(1500.f).MaxDesiredWidth(1200.f)
							[
								SNew(SBorder)
								.VAlign(VAlign_Fill).HAlign(HAlign_Fill)
								.BorderImage(FEditorStyle::GetBrush("ToolPanel.DarkGroupBorder"))
								[
									SAssignNew(VS_SCROLL_BOX,SScrollBox)
									.OnUserScrolled(this,&SKCS_MagicNodeWidget::OnVerticalScroll)
									.Orientation(EOrientation::Orient_Vertical)
									.ScrollBarThickness(FVector2D(8.f,8.f))
									+SScrollBox::Slot()
									[
										SNew(SHorizontalBox)
										+SHorizontalBox::Slot()
										.VAlign(VAlign_Fill).HAlign(HAlign_Left).AutoWidth()
										[
											SNew(SBorder)
											.VAlign(VAlign_Fill).HAlign(HAlign_Fill)
											.BorderImage(FEditorStyle::GetBrush("Graph.Node.Body"))
											[
												SAssignNew(LINE_COUNTER,SListView<TSharedPtr<FString>>)
												.OnSelectionChanged(this,&SKCS_MagicNodeWidget::OnSelectedLineCounterItem)
												.OnGenerateRow(this,&SKCS_MagicNodeWidget::OnGenerateLineCounter)
												.ScrollbarVisibility(EVisibility::Collapsed)
												.ListItemsSource(&LineCount).ItemHeight(14)
												.SelectionMode(ESelectionMode::Single)
											]
										]
										+SHorizontalBox::Slot()
										.VAlign(VAlign_Fill).HAlign(HAlign_Fill)
										[
											SAssignNew(SCRIPT_EDITOR,SKCS_TextEditorWidget)
											.OnTextChanged(this,&SKCS_MagicNodeWidget::OnScriptTextChanged,ETextCommit::Default)
											.OnTextCommitted(this,&SKCS_MagicNodeWidget::OnScriptTextComitted)
											.IsReadOnly(this,&SKCS_MagicNodeWidget::IsScriptSourceLocked)
											.OnInvokeSearch(this,&SKCS_MagicNodeWidget::OnInvokedSearch)
											.IsEnabled(this,&SKCS_MagicNodeWidget::HasScript)
											.Text(this,&SKCS_MagicNodeWidget::GetScriptText)
											.VScrollBar(VS_SCROLL).HScrollBar(HSS_SCROLL)
											.Marshaller(MARSHALL.ToSharedRef())
											.CanKeyboardFocus(true)
										]
									]
								]
							]
						]
						+SOverlay::Slot()
						.VAlign(VAlign_Top).HAlign(HAlign_Right)
						[
							SNew(SBox)
							.VAlign(VAlign_Fill).HAlign(HAlign_Fill)
							.WidthOverride(250.f).HeightOverride(80.f)
							.Visibility(this,&SKCS_MagicNodeWidget::GetSearchBoxVisibility)
							[
								SNew(SBorder)
								.VAlign(VAlign_Fill).HAlign(HAlign_Fill)
								.BorderImage(FEditorStyle::GetBrush("Sequencer.Thumbnail.SectionHandle"))
								[
									SNew(SBorder)
									.VAlign(VAlign_Fill).HAlign(HAlign_Fill)
									.BorderImage(FEditorStyle::GetBrush("Menu.Background"))
									[
										SNew(SVerticalBox)
										+SVerticalBox::Slot().Padding(5)
										.VAlign(VAlign_Top).HAlign(HAlign_Fill)
										[
											SNew(SHorizontalBox)
											+SHorizontalBox::Slot()
											.Padding(0,2,5,0).AutoWidth()
											.VAlign(VAlign_Top).HAlign(HAlign_Left)
											[
												SNew(SCheckBox)
												.OnCheckStateChanged(this,&SKCS_MagicNodeWidget::OnSearchSensitiveChanged)
												.IsChecked(this,&SKCS_MagicNodeWidget::IsSearchSensitive)
												.Style(FEditorStyle::Get(),"ToggleButtonCheckbox")
												.Content()
												[
													SNew(STextBlock).Margin(2)
													.Text(LOCTEXT("KCS_SearchToggleCase","Aa"))
													.ColorAndOpacity(FSlateColor(FLinearColor(FColor(255,255,255,225))))
													.Font(FKCS_NodeStyle::Get().Get()->GetWidgetStyle<FTextBlockStyle>("KCS.CodeBlockStyle").Font)
												]
											]
											+SHorizontalBox::Slot()
											.VAlign(VAlign_Fill).HAlign(HAlign_Fill)
											[
												SAssignNew(SEARCH_TEXT,SEditableTextBox)
												.OnTextChanged(this,&SKCS_MagicNodeWidget::OnSearchTextChanged,ETextCommit::Default)
												.OnTextCommitted(this,&SKCS_MagicNodeWidget::OnSearchTextComitted)
												.Text(this,&SKCS_MagicNodeWidget::GetSearchText)
												.SelectAllTextWhenFocused(true)
											]
											+SHorizontalBox::Slot()
											.Padding(5,2,0,0).AutoWidth()
											.VAlign(VAlign_Top).HAlign(HAlign_Left)
											[
												SNew(SButton)
												.OnClicked(this,&SKCS_MagicNodeWidget::OnClickedSearchGlass)
												.ButtonStyle(FEditorStyle::Get(),"NoBorder")
												[
													SNew(SImage)
													.Image(FEditorStyle::Get().GetBrush(TEXT("Symbols.SearchGlass")))
												]
											]
										]
										+SVerticalBox::Slot().Padding(5)
										.VAlign(VAlign_Top).HAlign(HAlign_Fill)
										[
											SNew(SHorizontalBox)
											+SHorizontalBox::Slot()
											.Padding(5,0,5,0).AutoWidth()
											.VAlign(VAlign_Top).HAlign(HAlign_Left)
											[
												SNew(SBox)
												.VAlign(VAlign_Fill).HAlign(HAlign_Fill)
												.WidthOverride(22.f).HeightOverride(22.f)
											]
											+SHorizontalBox::Slot()
											.VAlign(VAlign_Fill).HAlign(HAlign_Fill)
											[
												SNew(SEditableTextBox)
												.SelectAllTextWhenFocused(true)
												.Text(this,&SKCS_MagicNodeWidget::GetReplaceText)
												.OnTextCommitted(this,&SKCS_MagicNodeWidget::OnReplaceTextComitted)
											]
											+SHorizontalBox::Slot()
											.Padding(5,0,0,5).AutoWidth()
											.VAlign(VAlign_Top).HAlign(HAlign_Left)
											[
												SNew(SButton)
												.OnClicked(this,&SKCS_MagicNodeWidget::OnClickedReplaceSearch)
												.ButtonStyle(FEditorStyle::Get(),"NoBorder")
												[
													SNew(STextBlock).Margin(2)
													.Text(FText::FromString(FString(TEXT("\xf061"))))
													.Font(FEditorStyle::Get().GetFontStyle("FontAwesome.11"))
													.ColorAndOpacity(FSlateColor(FLinearColor(FColor(255,255,255,225))))
												]
											]
										]
									]
								]
							]
						]
					]
				]
				+SVerticalBox::Slot()
				.AutoHeight().Padding(10,0,10,0)
				[
					SNew(SVerticalBox)
					+SVerticalBox::Slot().AutoHeight()
					[
						HSS_SCROLL.ToSharedRef()
					]
				]
				+SVerticalBox::Slot()
				.AutoHeight().Padding(10,-5,10,5)
				[
					SNew(SBorder)
					.VAlign(VAlign_Fill).HAlign(HAlign_Fill)
					.BorderImage(FEditorStyle::GetBrush("Menu.Background"))
					[
						SNew(STextBlock).Margin(FMargin(5,0,0,0))
						.Text(this,&SKCS_MagicNodeWidget::GetCursorLocation)
						.ColorAndOpacity(FSlateColor(FLinearColor(FColor(225,225,255,225))))
					]
				]
				+SVerticalBox::Slot()
				.AutoHeight().Padding(10,5,10,5)
				[
					SNew(SHorizontalBox)
					.IsEnabled(this,&SKCS_MagicNodeWidget::HasScript)
					+SHorizontalBox::Slot().AutoWidth().Padding(0,0,5,0)
					[
						SNew(SButton)
						.ToolTipText(LOCTEXT("KCS_Compile","BUILD: Compiles node's binary data."))
						.OnClicked(this,&SKCS_MagicNodeWidget::OnClickedCompile)
						.IsEnabled(this,&SKCS_MagicNodeWidget::CanCompileScript)
						.ButtonStyle(FEditorStyle::Get(),"FlatButton.DarkGrey")
						.ForegroundColor(FSlateColor::UseForeground())
						.VAlign(VAlign_Center).HAlign(HAlign_Left)
						[
							SNew(SImage)
							.Image(this,&SKCS_MagicNodeWidget::GetCompilerIcon)
						]
					]
					+SHorizontalBox::Slot().AutoWidth().Padding(0,0,5,0)
					[
						SNew(SButton)
						.ToolTipText(LOCTEXT("KCS_ReloadScript","REFRESH: Rebuilds this node's pins (if there was a successful blueprint compilation)."))
						.OnClicked(this,&SKCS_MagicNodeWidget::OnClickedReloadScript)
						.IsEnabled(this,&SKCS_MagicNodeWidget::CanCompileScript)
						.ButtonStyle(FEditorStyle::Get(),"FlatButton.DarkGrey")
						.ForegroundColor(FSlateColor::UseForeground())
						.VAlign(VAlign_Center).HAlign(HAlign_Left)
						[
							SNew(SImage)
							.Image(FKCS_NodeStyle::Get()->GetBrush(TEXT("KCS.Toolbar.ReloadScript")))
						]
					]
					+SHorizontalBox::Slot().AutoWidth().Padding(0,0,5,0)
					[
						SNew(SButton)
						.ToolTipText(LOCTEXT("KCS_SaveScript","SAVE: Saves this node's source code (text file)."))
						.OnClicked(this,&SKCS_MagicNodeWidget::OnClickedSaveScript)
						.IsEnabled(this,&SKCS_MagicNodeWidget::CanCompileScript)
						.ButtonStyle(FEditorStyle::Get(),"FlatButton.DarkGrey")
						.ForegroundColor(FSlateColor::UseForeground())
						.VAlign(VAlign_Center).HAlign(HAlign_Left)
						[
							SNew(SImage)
							.Image(FKCS_NodeStyle::Get()->GetBrush(TEXT("KCS.Toolbar.SaveScript")))
						]
					]
				]
				+SVerticalBox::Slot()
				.AutoHeight().Padding(2,0,2,0)
				[
					SNew(SProgressBar)
					.BorderPadding(FVector2D::ZeroVector)
					.Percent(this,&SKCS_MagicNodeWidget::GetCompilerProgress)
					.FillColorAndOpacity(FSlateColor(FLinearColor(0.5f,0.5f,1.f)))
					.Visibility(this,&SKCS_MagicNodeWidget::GetCompilerProgressVisibility)
				]
			]
		]
	];
	//
	//
	UpdateTextEditorScriptReference();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SKCS_MagicNodeWidget::UpdateGraphNode() {
	SGraphNode::UpdateGraphNode();
	//
	SetLineCountList(GetLineCount());
}

void SKCS_MagicNodeWidget::UpdateTextEditorScriptReference() {
	UMagicNodeSharpSource* Script = (CastChecked<UKCS_MagicNode>(GraphNode))->GetScriptSource();
	//
	if (MARSHALL.IsValid()) {MARSHALL->SetScriptSource(Script);}
	if (SCRIPT_EDITOR.IsValid()) {SCRIPT_EDITOR->SetScriptSource(Script);}
}

void SKCS_MagicNodeWidget::UpdateTextEditorScriptReference(UMagicNodeSharpSource* Script) {
	if (SCRIPT_EDITOR.IsValid()) {SCRIPT_EDITOR->SetScriptSource(Script);}
	if (MARSHALL.IsValid()) {MARSHALL->SetScriptSource(Script);}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool SKCS_MagicNodeWidget::IsInteractable() const {
	return IsEnabled();
}

bool SKCS_MagicNodeWidget::SupportsKeyboardFocus() const {
	return true;
}

bool SKCS_MagicNodeWidget::HasScript() const {
	return (GetScriptSource()!=nullptr);
}

bool SKCS_MagicNodeWidget::IsCompiling() const {
	return (CastChecked<UKCS_MagicNode>(GraphNode))->IsCompiling();
}

bool SKCS_MagicNodeWidget::CanCompileScript() const {
	if (!HasScript()) {return false;}
	//
	return (CastChecked<UKCS_MagicNode>(GraphNode))->CanCompileScript();
}

bool SKCS_MagicNodeWidget::IsScriptSourceLocked() const {
	if (!HasScript()) {return true;}
	//
	UMagicNodeSharpSource* Script = (CastChecked<UKCS_MagicNode>(GraphNode))->GetScriptSource();
	//
	return Script->LockSourceCode;
}

bool SKCS_MagicNodeWidget::IsScriptSourceEditable() const {
	if (!HasScript()) {return false;}
	//
	UMagicNodeSharpSource* Script = (CastChecked<UKCS_MagicNode>(GraphNode))->GetScriptSource();
	//
	return !Script->LockSourceCode;
}

ECheckBoxState SKCS_MagicNodeWidget::IsSearchSensitive() const {
	return (SensitiveSearch) ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SKCS_MagicNodeWidget::SetScriptSource(UObject* New) {
	if (GraphNode==nullptr) {return;}
	//
	UMagicNodeSharpSource* Script = Cast<UMagicNodeSharpSource>(New);
	//
	CastChecked<UKCS_MagicNode>(GraphNode)->SetScriptSource(Script);
	CastChecked<UKCS_MagicNode>(GraphNode)->AdvancedPinDisplay = ENodeAdvancedPins::Hidden;;
	//
	UpdateTextEditorScriptReference(Script);
	//
	SetLineCountList(GetLineCount());
}

void SKCS_MagicNodeWidget::SetScriptText(const FText &NewText) {
	if (IsScriptSourceLocked()) {return;}
	//
	UMagicNodeSharpSource* Script = (CastChecked<UKCS_MagicNode>(GraphNode))->GetScriptSource();
	//
	if (Script->GetSource() != NewText.ToString()) {
		Script->Modify();
		//
		Script->SetSource(*NewText.ToString());
	}///
	//
	SetLineCountList(GetLineCount());
}

void SKCS_MagicNodeWidget::SetLineCountList(const int32 Count) {
	LineCount.Empty();
	//
	for (int32 I=0; I<=Count; I++) {
		FString ID = FString::Printf(TEXT("%i"),I+1);
		LineCount.Add(MakeShareable(new FString(ID)));
	}///
	//
	if (Count>0) {
		FString ID = FString::Printf(TEXT("%i"),Count+2);
		LineCount.Add(MakeShareable(new FString(ID)));
	}///
	//
	if (LINE_COUNTER.IsValid()) {LINE_COUNTER->RequestListRefresh();}
}

void SKCS_MagicNodeWidget::SetErrorMessage(const FCompilerResults &NewResult) {
	if (NewResult.Result==EMonoCompilerResult::Warning) {GraphNode->ErrorType=EMessageSeverity::Warning;}
	if (NewResult.Result==EMonoCompilerResult::Success) {GraphNode->ErrorType=EMessageSeverity::Info;}
	if (NewResult.Result==EMonoCompilerResult::Error) {GraphNode->ErrorType=EMessageSeverity::Error;}
	//
	if (NewResult.Result != EMonoCompilerResult::Success) {
		FSlateApplication::Get().SetKeyboardFocus(SCRIPT_EDITOR.ToSharedRef());
		SCRIPT_EDITOR->SetErrorType(NewResult.Result);
		SCRIPT_EDITOR->SetErrorLine(INDEX_NONE);
		//
		ErrorLine = NewResult.ErrorInfo.Line;
		int32 ErrorColumn = (NewResult.ErrorInfo.Column > 0) ? NewResult.ErrorInfo.Column-1 : 0;
		//
		SetLineCountList(GetLineCount());
		//
		if (ErrorLine > INDEX_NONE) {
			SCRIPT_EDITOR->SetErrorLine(ErrorLine-1);
			SCRIPT_EDITOR->GoToLineColumn(ErrorLine-1,ErrorColumn);
		} else {SCRIPT_EDITOR->GoToLineColumn(0,0);}
		//
		SetErrorMessage(NewResult.ErrorMessage);
	} else {
		SetLineCountList(GetLineCount());
		SetErrorMessage(FString());
	}///
}

void SKCS_MagicNodeWidget::SetErrorMessage(const FString &NewError) {
	ErrorMsg = NewError;
	//
	RefreshErrorInfo();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

UObject* SKCS_MagicNodeWidget::GetScriptSource() const {
	if (GraphNode==nullptr) {return nullptr;}
	//
	return (CastChecked<UKCS_MagicNode>(GraphNode))->GetScriptSource();
}

int32 SKCS_MagicNodeWidget::GetLineCount() const {
	if (!SCRIPT_EDITOR.IsValid()) {return 0;}
	//
	return SCRIPT_EDITOR->GetLineCount();
}

TOptional<float> SKCS_MagicNodeWidget::GetCompilerProgress() const {
	if (!HasScript()) {return TOptional<float>(0.f);}
	//
	return TOptional<float>(CompilerProgress);
}

FText SKCS_MagicNodeWidget::GetScriptText() const {
	if (!HasScript()) {return FText();}
	//
	UMagicNodeSharpSource* Script = (CastChecked<UKCS_MagicNode>(GraphNode))->GetScriptSource();
	//
	return FText::FromString(Script->GetSource());
}

FText SKCS_MagicNodeWidget::GetSearchText() const {
	if (!SCRIPT_EDITOR.IsValid()) {return FText::FromString(TEXT("Search.."));}
	//
	return SCRIPT_EDITOR->GetSearchText();
}

FText SKCS_MagicNodeWidget::GetReplaceText() const {
	return FText::FromString(ReplaceText);
}

FText SKCS_MagicNodeWidget::GetCursorLocation() const {
	if (!SCRIPT_EDITOR.IsValid()) {return FText();}
	//
	return FText::FromString(FString::Printf(TEXT("Ln: %i  |  Col: %i"),SCRIPT_EDITOR->GetCursorLocation().GetLineIndex()+1,SCRIPT_EDITOR->GetCursorLocation().GetOffset()+1));
}

EVisibility SKCS_MagicNodeWidget::GetSearchBoxVisibility() const {
	if (!HasScript()) {return EVisibility::Collapsed;}
	//
	return (ViewSearchBox) ? EVisibility::Visible : EVisibility::Collapsed;
}

EVisibility SKCS_MagicNodeWidget::GetScriptEditorVisibility() const {
	if (GraphNode==nullptr) {return EVisibility::Visible;}
	//
	if (GraphNode->AdvancedPinDisplay != ENodeAdvancedPins::Hidden) {return EVisibility::Visible;}
	//
	return EVisibility::Collapsed;
}

EVisibility SKCS_MagicNodeWidget::GetCompilerProgressVisibility() const {
	if (GraphNode==nullptr) {return EVisibility::Collapsed;}
	if (!HasScript()) {return EVisibility::Collapsed;}
	//
	if ((CastChecked<UKCS_MagicNode>(GraphNode))->IsCompiling()) {return EVisibility::Visible;}
	//
	return EVisibility::Collapsed;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const FSlateBrush* SKCS_MagicNodeWidget::GetCompilerIcon() const {
	if (!HasScript()) {return FEditorStyle::GetBrush(TEXT("Kismet.Status.Good"));}
	UPackage* Package = GetScriptSource()->GetOutermost();
	//
	if (Package->IsDirty() && GraphNode->ErrorType >= EMessageSeverity::Info) {
		return FEditorStyle::GetBrush(TEXT("Kismet.Status.Unknown"));
	} else if (GraphNode->ErrorType == EMessageSeverity::Warning) {
		return FEditorStyle::GetBrush(TEXT("Kismet.Status.Warning"));
	} else if (GraphNode->ErrorType <= EMessageSeverity::Error) {
		return FEditorStyle::GetBrush(TEXT("Kismet.Status.Error"));
	} else {return FKCS_NodeStyle::Get()->GetBrush(TEXT("KCS.Toolbar.CompileScript"));}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SKCS_MagicNodeWidget::OnScriptHorizontalScroll(float Offset) {}

void SKCS_MagicNodeWidget::OnVerticalScroll(float Offset) {
	VS_SCROLL->SetState(VS_SCROLL_BOX->GetScrollOffset(),VS_SCROLL_BOX->GetViewOffsetFraction());
}

void SKCS_MagicNodeWidget::OnInternalVerticalScroll(float Offset) {
	VS_SCROLL_BOX->SetScrollOffset(Offset);
}

void SKCS_MagicNodeWidget::OnScriptTextChanged(const FText &InText, ETextCommit::Type CommitType) {
	if (IsScriptSourceLocked()) {return;}
	//
	SetScriptText(InText);
	SetLineCountList(GetLineCount());
}

void SKCS_MagicNodeWidget::OnSearchTextChanged(const FText &InText, ETextCommit::Type CommitType) {
	SearchText = InText.ToString();
}

void SKCS_MagicNodeWidget::OnScriptTextComitted(const FText &NewText, ETextCommit::Type CommitInfo) {
	if (IsScriptSourceLocked()) {return;}
	//
	SetScriptText(NewText);
	SetLineCountList(GetLineCount());
}

void SKCS_MagicNodeWidget::OnSearchTextComitted(const FText &NewText, ETextCommit::Type CommitInfo) {
	SearchText.Empty(); SearchText.Append(NewText.ToString());
	//
	const ESearchCase::Type Case = (SensitiveSearch) ? ESearchCase::CaseSensitive : ESearchCase::IgnoreCase;
	SCRIPT_EDITOR->BeginSearch(FText::FromString(SearchText),Case,false);
}

void SKCS_MagicNodeWidget::OnReplaceTextComitted(const FText &NewText, ETextCommit::Type CommitInfo) {
	ReplaceText.Empty(); ReplaceText.Append(NewText.ToString());
}

void SKCS_MagicNodeWidget::OnInvokedSearch(bool DoSearch) {
	ReplaceText = TEXT("Replace..");
	ViewSearchBox = DoSearch;
	//
	if (ViewSearchBox) {
		FSlateApplication::Get().SetKeyboardFocus(SEARCH_TEXT.ToSharedRef());
	}///
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

FReply SKCS_MagicNodeWidget::OnClickedSearchGlass() {
	const ESearchCase::Type Case = (SensitiveSearch) ? ESearchCase::CaseSensitive : ESearchCase::IgnoreCase;
	if (!HasScript()) {return FReply::Unhandled();}
	//
	if (ReplaceText==TEXT("Replace..")) {ReplaceText=TEXT("");}
	//
	FSlateApplication::Get().SetKeyboardFocus(SCRIPT_EDITOR.ToSharedRef());
	SCRIPT_EDITOR->BeginSearch(FText::FromString(SearchText),Case,false);
	//
	return FReply::Handled();
}

void SKCS_MagicNodeWidget::OnSearchSensitiveChanged(ECheckBoxState NewState) {
	SensitiveSearch = (NewState==ECheckBoxState::Checked);
}

FReply SKCS_MagicNodeWidget::OnClickedReplaceSearch() {
	if (IsScriptSourceLocked()) {return FReply::Unhandled();}
	if (!SCRIPT_EDITOR.IsValid()) {return FReply::Unhandled();}
	//
	if (ReplaceText.IsEmpty()) {return FReply::Handled();}
	if (SearchText.IsEmpty()) {return FReply::Handled();}
	//
	//
	UMagicNodeSharpSource* Script = (CastChecked<UKCS_MagicNode>(GraphNode))->GetScriptSource();
	const FText DO = FText::FromString(FString(Script->GetSource()).Replace(*SearchText,*ReplaceText));
	//
	SCRIPT_EDITOR->BeginEditTransaction();
	 SCRIPT_EDITOR->SetText(DO);
	SCRIPT_EDITOR->EndEditTransaction();
	//
	return FReply::Handled();
}

FReply SKCS_MagicNodeWidget::OnClickedSaveScript() {
	if (IsScriptSourceLocked()) {return FReply::Unhandled();}
	//
	UMagicNodeSharpSource* Script = (CastChecked<UKCS_MagicNode>(GraphNode))->GetScriptSource();
	//
	if (GraphNode->AdvancedPinDisplay==ENodeAdvancedPins::Hidden) {
		(CastChecked<UKCS_MagicNode>(GraphNode))->SetTooltip(Script->GetSource());
	}///
	//
	UPackage* Package = Script->GetOutermost();
	TArray<UPackage*>PackagesToSave;
	PackagesToSave.Add(Package);
	//
	if (Package->IsDirty()) {
		FEditorFileUtils::PromptForCheckoutAndSave(PackagesToSave,true,false);
	}///
	//
	SetLineCountList(GetLineCount());
	//
	return FReply::Handled();
}

FReply SKCS_MagicNodeWidget::OnClickedReloadScript() {
	if (IsScriptSourceLocked()) {return FReply::Unhandled();}
	//
	CastChecked<UKCS_MagicNode>(GraphNode)->ReloadScript();
	SetLineCountList(GetLineCount());
	//
	return FReply::Handled();
}

FReply SKCS_MagicNodeWidget::OnClickedCompile() {
	if (IsScriptSourceLocked()) {return FReply::Unhandled();}
	//
	ErrorLine = INDEX_NONE;
	SetLineCountList(GetLineCount());
	SCRIPT_EDITOR->SetErrorLine(ErrorLine);
	SCRIPT_EDITOR->SetErrorType(EMonoCompilerResult::Success);
	//
	CastChecked<UKCS_MagicNode>(GraphNode)->CompileScript();
	//
	return FReply::Handled();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SKCS_MagicNodeWidget::OnSelectedLineCounterItem(TSharedPtr<FString>Item, ESelectInfo::Type SelectInfo) {
	if (!Item.IsValid()) {return;}
	//
	const int32 LineID = FCString::Atoi(**Item.Get());
	//
	FSlateApplication::Get().SetKeyboardFocus(SCRIPT_EDITOR.ToSharedRef());
	SCRIPT_EDITOR->GoToLineColumn(LineID-1,0);
	SCRIPT_EDITOR->SelectLine();
	//
	LINE_COUNTER->SetItemSelection(Item,false);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TSharedRef<ITableRow>SKCS_MagicNodeWidget::OnGenerateLineCounter(TSharedPtr<FString>Item, const TSharedRef<STableViewBase>&OwnerTable) {
	TSharedRef<SImage>ICON = SNew(SImage).Image(FKCS_NodeStyle::Get()->GetBrush(TEXT("KCS.Success")));
	//
	if (GraphNode->ErrorType==EMessageSeverity::Error) {ICON=SNew(SImage).Image(FKCS_NodeStyle::Get()->GetBrush(TEXT("KCS.Error")));}
	if (GraphNode->ErrorType==EMessageSeverity::Warning) {ICON=SNew(SImage).Image(FKCS_NodeStyle::Get()->GetBrush(TEXT("KCS.Warning")));}
	//
	if (FString::Printf(TEXT("%i"),ErrorLine)==(*Item.Get())) {
		return SNew(SComboRow<TSharedRef<FString>>,OwnerTable)
		[
			SNew(SBorder)
			.BorderBackgroundColor(FSlateColor(FLinearColor(1.f,0.1f,0.1f,1.f)))
			.BorderImage(FEditorStyle::GetBrush("Graph.Node.Body"))
			[
				SNew(SBox)
				.Padding(FMargin(0))
				.HAlign(HAlign_Left).VAlign(VAlign_Center)
				[
					ICON
				]
			]
		];//
	}///
	//
	return SNew(SComboRow<TSharedRef<FString>>,OwnerTable)
	[
		SNew(SBorder)
		.BorderImage(FEditorStyle::GetBrush("Graph.Node.Body"))
		.ColorAndOpacity(FLinearColor(1.f,1.f,1.f,1.f))
		.Padding(FMargin(5.f,0.f,5.f,0.f))
		[
			SNew(STextBlock)
			.Text(FText::FromString(*Item.Get()))
			.ColorAndOpacity(FSlateColor(FLinearColor(FColor(75,185,245,225))))
			.Font(FKCS_NodeStyle::Get().Get()->GetWidgetStyle<FTextBlockStyle>("KCS.CodeBlockStyle").Font)
		]
	];//
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

FReply SKCS_MagicNodeWidget::OnKeyDown(const FGeometry &Geometry, const FKeyEvent &KeyEvent) {
	if (!IsEnabled()||IsScriptSourceLocked()) {return FReply::Unhandled();}
	//
	SetLineCountList(GetLineCount());
	//
	if (KeyEvent.GetKey()==EKeys::Escape) {ViewSearchBox=false;}
	if (KeyEvent.GetKey()==EKeys::Enter) {return FReply::Handled();}
	//
	if (KeyEvent.GetKey()==EKeys::F && (KeyEvent.IsControlDown()||KeyEvent.IsCommandDown())) {
		return FReply::Handled();
	}///
	//
	if ((KeyEvent.GetKey()==EKeys::S)&&(KeyEvent.IsControlDown()||KeyEvent.IsCommandDown())) {
		OnClickedSaveScript(); return FReply::Handled();
	}///
	//
	return SGraphNode::OnKeyDown(Geometry,KeyEvent);
}

FReply SKCS_MagicNodeWidget::OnMouseMove(const FGeometry &Geometry, const FPointerEvent &MouseEvent) {
	if (!SCRIPT_EDITOR.IsValid()) {return SGraphNode::OnMouseMove(Geometry,MouseEvent);}
	if (!ErrorMsg.IsEmpty()) {return SGraphNode::OnMouseMove(Geometry,MouseEvent);}
	if (GraphNode==nullptr) {return SGraphNode::OnMouseMove(Geometry,MouseEvent);}
	if (HintTimer<=8.f) {return SGraphNode::OnMouseMove(Geometry,MouseEvent);}
	if (!IsEnabled()) {return SGraphNode::OnMouseMove(Geometry,MouseEvent);}
	//
	UKCS_MagicNode* KNode = (Cast<UKCS_MagicNode>(GraphNode));
	if (KNode==nullptr) {return SGraphNode::OnMouseMove(Geometry,MouseEvent);}
	//
	UMagicNodeSharpSource* Script = KNode->GetScriptSource();
	if (Script==nullptr) {return SGraphNode::OnMouseMove(Geometry,MouseEvent);}
	//
	if (KNode->AdvancedPinDisplay==ENodeAdvancedPins::Hidden) {
		KNode->SetTooltip(Script->GetSource());
		//
		return SGraphNode::OnMouseMove(Geometry,MouseEvent);
	} else if (SCRIPT_EDITOR->HasSuggestion()) {
		const FString &KeywordInfo = SCRIPT_EDITOR->GetKeywordInfo();
		//
		if (!KeywordInfo.IsEmpty()) {
			KNode->SetTooltip(KeywordInfo);
		} else {KNode->SetTooltip(TEXT(""));}
		//
		return SGraphNode::OnMouseMove(Geometry,MouseEvent);
	}///
	//
	if ((HasKeyboardFocus()||HasFocusedDescendants())&&(SCRIPT_EDITOR->GetCursorLocation().GetOffset()>=1)&&(GraphNode->AdvancedPinDisplay!=ENodeAdvancedPins::Hidden)) {
		FString Context = SCRIPT_EDITOR->GetAutoCompleteSubject();
		const FString Keyword = SCRIPT_EDITOR->GetUnderCursor();
		//
		if (!LastHint.Equals(Keyword,ESearchCase::CaseSensitive)) {LastHint=Keyword;}
		else {return SGraphNode::OnMouseMove(Geometry,MouseEvent);}
		//
		if (SCRIPT_EDITOR->IsAutoCompleteTree(Context)) {
			TArray<FString>Tree = SCRIPT_EDITOR->ParseAutoCompleteTree(Context);
			//
			const FMonoClassDefinition &ClassInfo = IKCS_MonoAnalyzer::GetClassInfo(Script,Tree,Keyword);
			if (ClassInfo.IsValid()) {KNode->SetTooltip(FString::Printf(TEXT("(%s)   %s\n\n%s"),*ClassInfo.ScopeToString(),*ClassInfo.Name.ToString(),*ClassInfo.Description)); {goto END;}}
		} else if (SCRIPT_EDITOR->IsAutoCompleteTree(Keyword)) {
			TArray<FString>Tree = SCRIPT_EDITOR->ParseAutoCompleteTree(Keyword);
			FString Key = Tree.Pop();
			//
			const FMonoClassDefinition &ClassInfo = IKCS_MonoAnalyzer::GetClassInfo(Script,Tree,Key);
			if (ClassInfo.IsValid()) {KNode->SetTooltip(FString::Printf(TEXT("(%s)   %s\n\n%s"),*ClassInfo.ScopeToString(),*ClassInfo.Name.ToString(),*ClassInfo.Description)); {goto END;}}
		} else {
			const FMonoClassDefinition &ClassInfo = IKCS_MonoAnalyzer::GetClassInfo(Script,Context,Keyword);
			if (ClassInfo.IsValid()) {KNode->SetTooltip(FString::Printf(TEXT("(%s)   %s\n\n%s"),*ClassInfo.ScopeToString(),*ClassInfo.Name.ToString(),*ClassInfo.Description)); {goto END;}}
		}///
		//
		if (SCRIPT_EDITOR->IsAutoCompleteTree(Context)) {
			TArray<FString>Tree = SCRIPT_EDITOR->ParseAutoCompleteTree(Context);
			//
			const FMonoFieldDefinition &FieldInfo = IKCS_MonoAnalyzer::GetFieldInfo(Script,Tree,Keyword);
			if (FieldInfo.IsValid()) {KNode->SetTooltip(FString::Printf(TEXT("%s(%s)   %s\n\n%s"),*FieldInfo.StackToString(),*FieldInfo.TypeToString(),*FieldInfo.Name.ToString(),*FieldInfo.Description)); {goto END;}}
		} else if (SCRIPT_EDITOR->IsAutoCompleteTree(Keyword)) {
			TArray<FString>Tree = SCRIPT_EDITOR->ParseAutoCompleteTree(Keyword);
			FString Key = Tree.Pop();
			//
			const FMonoFieldDefinition &FieldInfo = IKCS_MonoAnalyzer::GetFieldInfo(Script,Tree,Key);
			if (FieldInfo.IsValid()) {KNode->SetTooltip(FString::Printf(TEXT("%s(%s)   %s\n\n%s"),*FieldInfo.StackToString(),*FieldInfo.TypeToString(),*FieldInfo.Name.ToString(),*FieldInfo.Description)); {goto END;}}
		} else {
			const FMonoFieldDefinition &FieldInfo = IKCS_MonoAnalyzer::GetFieldInfo(Script,Context,Keyword);
			if (FieldInfo.IsValid()) {KNode->SetTooltip(FString::Printf(TEXT("%s(%s)   %s\n\n%s"),*FieldInfo.StackToString(),*FieldInfo.TypeToString(),*FieldInfo.Name.ToString(),*FieldInfo.Description)); {goto END;}}
		}///
		//
		if (SCRIPT_EDITOR->IsAutoCompleteTree(Context)) {
			TArray<FString>Tree = SCRIPT_EDITOR->ParseAutoCompleteTree(Context);
			//
			const FMonoFieldDefinition &PropInfo = IKCS_MonoAnalyzer::GetPropertyInfo(Script,Tree,Keyword);
			if (PropInfo.IsValid()) {KNode->SetTooltip(FString::Printf(TEXT("%s(%s)   %s\n\n%s"),*PropInfo.StackToString(),*PropInfo.TypeToString(),*PropInfo.Name.ToString(),*PropInfo.Description)); {goto END;}}
		} else if (SCRIPT_EDITOR->IsAutoCompleteTree(Keyword)) {
			TArray<FString>Tree = SCRIPT_EDITOR->ParseAutoCompleteTree(Keyword);
			FString Key = Tree.Pop();
			//
			const FMonoFieldDefinition &PropInfo = IKCS_MonoAnalyzer::GetPropertyInfo(Script,Tree,Key);
			if (PropInfo.IsValid()) {KNode->SetTooltip(FString::Printf(TEXT("%s(%s)   %s\n\n%s"),*PropInfo.StackToString(),*PropInfo.TypeToString(),*PropInfo.Name.ToString(),*PropInfo.Description)); {goto END;}}
		} else {
			const FMonoFieldDefinition &PropInfo = IKCS_MonoAnalyzer::GetPropertyInfo(Script,Context,Keyword);
			if (PropInfo.IsValid()) {KNode->SetTooltip(FString::Printf(TEXT("%s(%s)   %s\n\n%s"),*PropInfo.StackToString(),*PropInfo.TypeToString(),*PropInfo.Name.ToString(),*PropInfo.Description)); {goto END;}}
		}///
		//
		if (SCRIPT_EDITOR->IsAutoCompleteTree(Context)) {
			TArray<FString>Tree = SCRIPT_EDITOR->ParseAutoCompleteTree(Context);
			//
			const FMonoMethodDefinition &MethodInfo = IKCS_MonoAnalyzer::GetMethodInfo(Script,Tree,Keyword);
			if (MethodInfo.IsValid()) {KNode->SetTooltip(FString::Printf(TEXT("(%s)   %s\n\n%s"),*MethodInfo.TypeToString(),*MethodInfo.Name.ToString(),*MethodInfo.Description)); {goto END;}}
		} else if (SCRIPT_EDITOR->IsAutoCompleteTree(Keyword)) {
			TArray<FString>Tree = SCRIPT_EDITOR->ParseAutoCompleteTree(Keyword);
			FString Key = Tree.Pop();
			//
			const FMonoMethodDefinition &MethodInfo = IKCS_MonoAnalyzer::GetMethodInfo(Script,Tree,Key);
			if (MethodInfo.IsValid()) {KNode->SetTooltip(FString::Printf(TEXT("(%s)   %s\n\n%s"),*MethodInfo.TypeToString(),*MethodInfo.Name.ToString(),*MethodInfo.Description)); {goto END;}}
		} else {
			const FMonoMethodDefinition &MethodInfo = IKCS_MonoAnalyzer::GetMethodInfo(Script,Context,Keyword);
			if (MethodInfo.IsValid()) {KNode->SetTooltip(FString::Printf(TEXT("(%s)   %s\n\n%s"),*MethodInfo.TypeToString(),*MethodInfo.Name.ToString(),*MethodInfo.Description)); {goto END;}}
		}///
	} else if (GraphNode->AdvancedPinDisplay!=ENodeAdvancedPins::Hidden) {
		KNode->SetTooltip(TEXT(""));
	}///
	//
	END: return SGraphNode::OnMouseMove(Geometry,MouseEvent);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#undef LOCTEXT_NAMESPACE

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////