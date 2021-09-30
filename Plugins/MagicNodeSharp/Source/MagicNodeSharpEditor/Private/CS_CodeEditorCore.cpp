//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
///			Copyright 2021 (C) Bruno Xavier B. Leite
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "CS_CodeEditorCore.h"
#include "CS_EditorStyle.h"

#include "KCS_NodeStyle.h"
#include "KCS_NodeWidget.h"
#include "KCS_MonoAnalyzer.h"
#include "KCS_TextSyntaxHighlighter.h"

#include "Runtime/SlateCore/Public/Widgets/SOverlay.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define LOCTEXT_NAMESPACE "Synaptech"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

SCS_CodeEditorCore::SCS_CodeEditorCore() {}
SCS_CodeEditorCore::~SCS_CodeEditorCore(){}

void SCS_CodeEditorCore::Construct(const FArguments &InArgs, UMagicNodeSharpSource* InScriptSource) {
	MARSHALL = FTextSyntaxHighlighter::Create(
		FTextSyntaxHighlighter::FSyntaxTextStyle()
	);///
	//
	if (MARSHALL.IsValid()) {MARSHALL->SetScriptSource(InScriptSource);}
	//
	ZeroLocation = FTextLocation{};
	ScriptSource = InScriptSource;
	check(InScriptSource);
	//
	//
	ErrorType = EMonoCompilerResult::Success;
	ErrorLine = INDEX_NONE;
	ViewSearchBox = false;
	//
	//
	VS_SCROLL = SNew(SScrollBar)
	.OnUserScrolled(this,&SCS_CodeEditorCore::OnInternalVerticalScroll)
	.Thickness(FVector2D(8.f,8.f)).AlwaysShowScrollbar(false)
	.Orientation(Orient_Vertical);
	//
	HS_SCROLL = SNew(SScrollBar)
	.OnUserScrolled(this,&SCS_CodeEditorCore::OnScriptHorizontalScroll)
	.Orientation(Orient_Horizontal).AlwaysShowScrollbar(true)
	.Thickness(FVector2D(8.f,8.f));
	//
	//
	TSharedPtr<SOverlay>OverlayWidget;
	//
	ChildSlot[
	SNew(SHorizontalBox)
	+SHorizontalBox::Slot()
	.VAlign(VAlign_Fill).HAlign(HAlign_Fill)
	[
		SAssignNew(OverlayWidget,SOverlay)
		+SOverlay::Slot()
		[
			SNew(SVerticalBox)
			+SVerticalBox::Slot()
			[
				SNew(SBox)
				.VAlign(VAlign_Fill).HAlign(HAlign_Fill)
				.MinDesiredWidth(500.f).MinDesiredHeight(300.f)
				[
					SNew(SBorder)
					.VAlign(VAlign_Fill).HAlign(HAlign_Fill)
					.BorderImage(FEditorStyle::GetBrush("ToolPanel.DarkGroupBorder"))
					[
						SAssignNew(VS_SCROLL_BOX,SScrollBox)
						.OnUserScrolled(this,&SCS_CodeEditorCore::OnVerticalScroll)
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
									.OnSelectionChanged(this,&SCS_CodeEditorCore::OnSelectedLineCounterItem)
									.OnGenerateRow(this,&SCS_CodeEditorCore::OnGenerateLineCounter)
									.ScrollbarVisibility(EVisibility::Collapsed)
									.ListItemsSource(&LineCount).ItemHeight(14)
									.SelectionMode(ESelectionMode::Single)
								]
							]
							+SHorizontalBox::Slot()
							.VAlign(VAlign_Fill).HAlign(HAlign_Fill)
							[
								SAssignNew(SCRIPT_EDITOR,SKCS_TextEditorWidget)
								.OnTextChanged(this,&SCS_CodeEditorCore::OnScriptTextChanged,ETextCommit::Default)
								.OnTextCommitted(this,&SCS_CodeEditorCore::OnScriptTextComitted)
								.OnInvokeSearch(this,&SCS_CodeEditorCore::OnInvokedSearch)
								.IsReadOnly(this,&SCS_CodeEditorCore::IsSourceLocked)
								.IsEnabled(this,&SCS_CodeEditorCore::HasScript)
								.Text(this,&SCS_CodeEditorCore::GetScriptText)
								.VScrollBar(VS_SCROLL).HScrollBar(HS_SCROLL)
								.Marshaller(MARSHALL.ToSharedRef())
								.CanKeyboardFocus(true)
							]
						]
					]
				]
			]
			+SVerticalBox::Slot()
			.AutoHeight().Padding(0,5,0,0)
			[
				SNew(SBorder)
				.VAlign(VAlign_Fill).HAlign(HAlign_Fill)
				.BorderImage(FEditorStyle::GetBrush("Menu.Background"))
				[
					SNew(STextBlock).Margin(FMargin(5,0,0,0))
					.Text(this,&SCS_CodeEditorCore::GetCursorLocation)
					.ColorAndOpacity(FSlateColor(FLinearColor(FColor(225,225,255,225))))
				]
			]
		]
		+SOverlay::Slot().Padding(14,4)
		.VAlign(VAlign_Top).HAlign(HAlign_Right)
		[
			SNew(SBox)
			.VAlign(VAlign_Fill).HAlign(HAlign_Fill)
			.WidthOverride(250.f).HeightOverride(80.f)
			.Visibility(this,&SCS_CodeEditorCore::GetSearchBoxVisibility)
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
								.OnCheckStateChanged(this,&SCS_CodeEditorCore::OnSearchSensitiveChanged)
								.IsChecked(this,&SCS_CodeEditorCore::IsSearchSensitive)
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
								.OnTextChanged(this,&SCS_CodeEditorCore::OnSearchTextChanged,ETextCommit::Default)
								.OnTextCommitted(this,&SCS_CodeEditorCore::OnSearchTextComitted)
								.Text(this,&SCS_CodeEditorCore::GetSearchText)
								.SelectAllTextWhenFocused(true)
							]
							+SHorizontalBox::Slot()
							.Padding(5,2,0,0).AutoWidth()
							.VAlign(VAlign_Top).HAlign(HAlign_Left)
							[
								SNew(SButton)
								.OnClicked(this,&SCS_CodeEditorCore::OnClickedSearchGlass)
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
								.Text(this,&SCS_CodeEditorCore::GetReplaceText)
								.OnTextCommitted(this,&SCS_CodeEditorCore::OnReplaceTextComitted)
							]
							+SHorizontalBox::Slot()
							.Padding(5,0,0,5).AutoWidth()
							.VAlign(VAlign_Top).HAlign(HAlign_Left)
							[
								SNew(SButton)
								.OnClicked(this,&SCS_CodeEditorCore::OnClickedReplaceSearch)
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
		+SOverlay::Slot().Padding(40,28)
		.VAlign(VAlign_Bottom).HAlign(HAlign_Left)
		[
			SNew(SBorder)
			.VAlign(VAlign_Bottom).HAlign(HAlign_Left)
			.Visibility(this,&SCS_CodeEditorCore::GetHintBoxVisibility)
			.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
			[
				SNew(STextBlock).Margin(4)
				.Text(this,&SCS_CodeEditorCore::GetHintKeywordInfo)
				.Font(FCoreStyle::GetDefaultFontStyle("Bold",12))
				.ColorAndOpacity(FSlateColor(FLinearColor(FColor(255,255,255,225))))
			]
		]
	]
	];
	//
	//
	UpdateTextEditorScriptReference();
	SetLineCountList(GetLineCount());
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SCS_CodeEditorCore::Tick(const FGeometry &AllottedGeometry, const double CurrentTime, const float DeltaTime) {
	HintTimer = (HintTimer>=16.f) ? 0.f : (HintTimer+1.f);
	//
	SCompoundWidget::Tick(AllottedGeometry,CurrentTime,DeltaTime);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

FReply SCS_CodeEditorCore::OnKeyDown(const FGeometry &Geometry, const FKeyEvent &KeyEvent) {
	if (KeyEvent.GetKey()==EKeys::Escape) {
		ViewSearchBox = false; KeywordInfo.Empty();
	}///
	//
	return SCompoundWidget::OnKeyDown(Geometry,KeyEvent);
}

FReply SCS_CodeEditorCore::OnMouseMove(const FGeometry &Geometry, const FPointerEvent &MouseEvent) {
	if (!SCRIPT_EDITOR.IsValid()) {return SCompoundWidget::OnMouseMove(Geometry,MouseEvent);}
	if (ScriptSource==nullptr) {return SCompoundWidget::OnMouseMove(Geometry,MouseEvent);}
	if (HintTimer<=8.f) {return SCompoundWidget::OnMouseMove(Geometry,MouseEvent);}
	//
	if ((HasKeyboardFocus()||HasFocusedDescendants())&&(SCRIPT_EDITOR->GetCursorLocation().GetOffset()>=1)) {
		FString Context = SCRIPT_EDITOR->GetAutoCompleteSubject();
		FString Keyword = SCRIPT_EDITOR->GetUnderCursor();
		//
		if (SCRIPT_EDITOR->IsAutoCompleteTree(Context)) {
			const FMonoNamespaceDefinition &SpaceInfo = IKCS_MonoAnalyzer::GetNamespaceInfo(ScriptSource,Context);
			//
			if (SpaceInfo.IsValid()) {
				FString Namespace{};
				//
				for (const auto &N : SpaceInfo.Namespaces) {
					Namespace.Append(N+TEXT("."));
				} Namespace.RemoveFromEnd(TEXT("."));
				//
				KeywordInfo = FString::Printf(TEXT("(%s)\n\n%s"),*SpaceInfo.Name.ToString(),*Namespace); {goto END;}
			}///
		} else if (SCRIPT_EDITOR->IsAutoCompleteTree(Keyword)) {
			const FMonoNamespaceDefinition &SpaceInfo = IKCS_MonoAnalyzer::GetNamespaceInfo(ScriptSource,Keyword);
			//
			if (SpaceInfo.IsValid()) {
				FString Namespace{};
				//
				for (const auto &N : SpaceInfo.Namespaces) {
					Namespace.Append(N+TEXT("."));
				} Namespace.RemoveFromEnd(TEXT("."));
				//
				KeywordInfo = FString::Printf(TEXT("(%s)\n\n%s"),*SpaceInfo.Name.ToString(),*Namespace); {goto END;}
			}///
		} else if (SCRIPT_EDITOR->IsAutoCompleteTree(Context)) {
			TArray<FString>Tree{};
			//
			Context.RemoveFromEnd(TEXT("."));
			for (const auto &Item : SCRIPT_EDITOR->ParseAutoCompleteTree(Context)) {
				Tree.Add(SCRIPT_EDITOR->FilterKeyword(Item));
			}///
			//
			const FMonoClassDefinition &ClassInfo = IKCS_MonoAnalyzer::GetClassInfo(ScriptSource,Tree,Keyword);
			if (ClassInfo.IsValid()) {KeywordInfo=FString::Printf(TEXT("(%s)   %s\n\n%s"),*ClassInfo.ScopeToString(),*ClassInfo.Name.ToString(),*ClassInfo.Description); {goto END;}}
		} else if (SCRIPT_EDITOR->IsAutoCompleteTree(Keyword)) {
			TArray<FString>Tree{};
			//
			Keyword.RemoveFromEnd(TEXT("."));
			for (const auto &Item : SCRIPT_EDITOR->ParseAutoCompleteTree(Keyword)) {
				Tree.Add(SCRIPT_EDITOR->FilterKeyword(Item));
			} const FString Key = Tree.Pop();
			//
			const FMonoClassDefinition &ClassInfo = IKCS_MonoAnalyzer::GetClassInfo(ScriptSource,Tree,Key);
			if (ClassInfo.IsValid()) {KeywordInfo=FString::Printf(TEXT("(%s)   %s\n\n%s"),*ClassInfo.ScopeToString(),*ClassInfo.Name.ToString(),*ClassInfo.Description); {goto END;}}
		} else {
			const FMonoClassDefinition &ClassInfo = IKCS_MonoAnalyzer::GetClassInfo(ScriptSource,Context,Keyword);
			if (ClassInfo.IsValid()) {KeywordInfo=FString::Printf(TEXT("(%s)   %s\n\n%s"),*ClassInfo.ScopeToString(),*ClassInfo.Name.ToString(),*ClassInfo.Description); {goto END;}}
		}///
		//
		if (SCRIPT_EDITOR->IsAutoCompleteTree(Context)) {
			TArray<FString>Tree{};
			//
			Context.RemoveFromEnd(TEXT("."));
			for (const auto &Item : SCRIPT_EDITOR->ParseAutoCompleteTree(Context)) {
				Tree.Add(SCRIPT_EDITOR->FilterKeyword(Item));
			}///
			//
			const FMonoFieldDefinition &FieldInfo = IKCS_MonoAnalyzer::GetFieldInfo(ScriptSource,Tree,Keyword);
			if (FieldInfo.IsValid()) {KeywordInfo=FString::Printf(TEXT("%s(%s)   %s\n\n%s"),*FieldInfo.StackToString(),*FieldInfo.TypeToString(),*FieldInfo.Name.ToString(),*FieldInfo.Description); {goto END;}}
		} else if (SCRIPT_EDITOR->IsAutoCompleteTree(Keyword)) {
			TArray<FString>Tree{};
			//
			Keyword.RemoveFromEnd(TEXT("."));
			for (const auto &Item : SCRIPT_EDITOR->ParseAutoCompleteTree(Keyword)) {
				Tree.Add(SCRIPT_EDITOR->FilterKeyword(Item));
			} const FString Key = Tree.Pop();
			//
			const FMonoFieldDefinition &FieldInfo = IKCS_MonoAnalyzer::GetFieldInfo(ScriptSource,Tree,Key);
			if (FieldInfo.IsValid()) {KeywordInfo=FString::Printf(TEXT("%s(%s)   %s\n\n%s"),*FieldInfo.StackToString(),*FieldInfo.TypeToString(),*FieldInfo.Name.ToString(),*FieldInfo.Description); {goto END;}}
		} else {
			const FMonoFieldDefinition &FieldInfo = IKCS_MonoAnalyzer::GetFieldInfo(ScriptSource,Context,Keyword);
			if (FieldInfo.IsValid()) {KeywordInfo=FString::Printf(TEXT("%s(%s)   %s\n\n%s"),*FieldInfo.StackToString(),*FieldInfo.TypeToString(),*FieldInfo.Name.ToString(),*FieldInfo.Description); {goto END;}}
		}///
		//
		if (SCRIPT_EDITOR->IsAutoCompleteTree(Context)) {
			TArray<FString>Tree{};
			//
			Context.RemoveFromEnd(TEXT("."));
			for (const auto &Item : SCRIPT_EDITOR->ParseAutoCompleteTree(Context)) {
				Tree.Add(SCRIPT_EDITOR->FilterKeyword(Item));
			}///
			//
			const FMonoFieldDefinition &PropInfo = IKCS_MonoAnalyzer::GetPropertyInfo(ScriptSource,Tree,Keyword);
			if (PropInfo.IsValid()) {KeywordInfo=FString::Printf(TEXT("%s(%s)   %s\n\n%s"),*PropInfo.StackToString(),*PropInfo.TypeToString(),*PropInfo.Name.ToString(),*PropInfo.Description); {goto END;}}
		} else if (SCRIPT_EDITOR->IsAutoCompleteTree(Keyword)) {
			TArray<FString>Tree{};
			//
			Keyword.RemoveFromEnd(TEXT("."));
			for (const auto &Item : SCRIPT_EDITOR->ParseAutoCompleteTree(Keyword)) {
				Tree.Add(SCRIPT_EDITOR->FilterKeyword(Item));
			} const FString Key = Tree.Pop();
			//
			const FMonoFieldDefinition &PropInfo = IKCS_MonoAnalyzer::GetPropertyInfo(ScriptSource,Tree,Key);
			if (PropInfo.IsValid()) {KeywordInfo=FString::Printf(TEXT("%s(%s)   %s\n\n%s"),*PropInfo.StackToString(),*PropInfo.TypeToString(),*PropInfo.Name.ToString(),*PropInfo.Description); {goto END;}}
		} else {
			const FMonoFieldDefinition &PropInfo = IKCS_MonoAnalyzer::GetPropertyInfo(ScriptSource,Context,Keyword);
			if (PropInfo.IsValid()) {KeywordInfo=FString::Printf(TEXT("%s(%s)   %s\n\n%s"),*PropInfo.StackToString(),*PropInfo.TypeToString(),*PropInfo.Name.ToString(),*PropInfo.Description); {goto END;}}
		}///
		//
		if (SCRIPT_EDITOR->IsAutoCompleteTree(Context)) {
			TArray<FString>Tree{};
			//
			Context.RemoveFromEnd(TEXT("."));
			for (const auto &Item : SCRIPT_EDITOR->ParseAutoCompleteTree(Context)) {
				Tree.Add(SCRIPT_EDITOR->FilterKeyword(Item));
			}///
			//
			const FMonoMethodDefinition &MethodInfo = IKCS_MonoAnalyzer::GetMethodInfo(ScriptSource,Tree,Keyword);
			if (MethodInfo.IsValid()) {KeywordInfo=FString::Printf(TEXT("(%s)   %s\n\n%s"),*MethodInfo.TypeToString(),*MethodInfo.Name.ToString(),*MethodInfo.Description); {goto END;}}
		} else if (SCRIPT_EDITOR->IsAutoCompleteTree(Keyword)) {
			TArray<FString>Tree{};
			//
			Keyword.RemoveFromEnd(TEXT("."));
			for (const auto &Item : SCRIPT_EDITOR->ParseAutoCompleteTree(Keyword)) {
				Tree.Add(SCRIPT_EDITOR->FilterKeyword(Item));
			} const FString Key = Tree.Pop();
			//
			const FMonoMethodDefinition &MethodInfo = IKCS_MonoAnalyzer::GetMethodInfo(ScriptSource,Tree,Key);
			if (MethodInfo.IsValid()) {KeywordInfo=FString::Printf(TEXT("(%s)   %s\n\n%s"),*MethodInfo.TypeToString(),*MethodInfo.Name.ToString(),*MethodInfo.Description); {goto END;}}
		} else {
			const FMonoMethodDefinition &MethodInfo = IKCS_MonoAnalyzer::GetMethodInfo(ScriptSource,Context,Keyword);
			if (MethodInfo.IsValid()) {KeywordInfo=FString::Printf(TEXT("(%s)   %s\n\n%s"),*MethodInfo.TypeToString(),*MethodInfo.Name.ToString(),*MethodInfo.Description); {goto END;}}
		}///
	}///
	//
	if (SCRIPT_EDITOR->HasSuggestion()) {
		const FString &Info = SCRIPT_EDITOR->GetKeywordInfo();
		if (!Info.IsEmpty()) {KeywordInfo=SCRIPT_EDITOR->GetKeywordInfo();}
	}///
	//
	END: return SCompoundWidget::OnMouseMove(Geometry,MouseEvent);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool SCS_CodeEditorCore::HasScript() const {
	return (ScriptSource!=nullptr);
}

bool SCS_CodeEditorCore::IsSourceLocked() const {
	return ScriptSource->LockSourceCode;
}

bool SCS_CodeEditorCore::IsScriptEditable() const {
	return !ScriptSource->LockSourceCode;
}

int32 SCS_CodeEditorCore::GetLineCount() const {
	if (!SCRIPT_EDITOR.IsValid()) {return 0;}
	//
	return SCRIPT_EDITOR->GetLineCount();
}

FText SCS_CodeEditorCore::GetScriptText() const {
	return FText::FromString(ScriptSource->GetSource());
}

FText SCS_CodeEditorCore::GetSearchText() const {
	if (!SCRIPT_EDITOR.IsValid()) {return FText::FromString(TEXT("Search.."));}
	//
	return SCRIPT_EDITOR->GetSearchText();
}

FText SCS_CodeEditorCore::GetReplaceText() const {
	return FText::FromString(ReplaceText);
}

FText SCS_CodeEditorCore::GetHintKeywordInfo() const {
	return FText::FromString(KeywordInfo);
}

const FTextLocation &SCS_CodeEditorCore::GetCursorOffset() const {
	if (!SCRIPT_EDITOR.IsValid()) {return ZeroLocation;}
	//
	return SCRIPT_EDITOR->GetCursorLocation();
}

FText SCS_CodeEditorCore::GetCursorLocation() const {
	if (!SCRIPT_EDITOR.IsValid()) {return FText{};}
	//
	return FText::FromString(FString::Printf(TEXT("Ln: %i  |  Col: %i"),SCRIPT_EDITOR->GetCursorLocation().GetLineIndex()+1,SCRIPT_EDITOR->GetCursorLocation().GetOffset()+1));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SCS_CodeEditorCore::SetLineCountList(const int32 Count) {
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

void SCS_CodeEditorCore::SetScriptText(const FText &NewText) {
	if (IsSourceLocked()) {return;}
	//
	if (ScriptSource->GetSource() != NewText.ToString()) {
		ScriptSource->Modify();
		ScriptSource->SetSource(*NewText.ToString());
	}///
	//
	SetLineCountList(GetLineCount());
}

void SCS_CodeEditorCore::SetScriptError(const EMonoCompilerResult &NewResult, const FSourceInfo &Info) {
	FSlateApplication::Get().SetKeyboardFocus(SCRIPT_EDITOR.ToSharedRef());
	//
	int32 ErrorColumn = (Info.Column > 0) ? Info.Column-1 : 0;
	ErrorLine = Info.Line; ErrorType = NewResult;
	//
	SCRIPT_EDITOR->SetErrorType(ErrorType);
	SCRIPT_EDITOR->SetErrorLine(ErrorLine);
	//
	SetLineCountList(GetLineCount());
	//
	if (ErrorLine > INDEX_NONE) {
		SCRIPT_EDITOR->SetErrorLine(ErrorLine-1);
		SCRIPT_EDITOR->GoToLineColumn(ErrorLine-1,ErrorColumn);
	} else {SCRIPT_EDITOR->GoToLineColumn(0,0);}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ECheckBoxState SCS_CodeEditorCore::IsSearchSensitive() const {
	return (SensitiveSearch) ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

EVisibility SCS_CodeEditorCore::GetSearchBoxVisibility() const {
	if (!HasScript()) {return EVisibility::Collapsed;}
	//
	return (ViewSearchBox) ? EVisibility::Visible : EVisibility::Collapsed;
}

EVisibility SCS_CodeEditorCore::GetHintBoxVisibility() const {
	if (!HasScript()) {return EVisibility::Collapsed;}
	//
	return (KeywordInfo.IsEmpty()) ? EVisibility::Collapsed : EVisibility::Visible;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

FReply SCS_CodeEditorCore::OnClickedSearchGlass() {
	if (!HasScript()) {return FReply::Unhandled();}
	//
	const ESearchCase::Type Case = (SensitiveSearch) ? ESearchCase::CaseSensitive : ESearchCase::IgnoreCase;
	if (ReplaceText==TEXT("Replace..")) {ReplaceText=TEXT("");}
	//
	FSlateApplication::Get().SetKeyboardFocus(SCRIPT_EDITOR.ToSharedRef());
	SCRIPT_EDITOR->BeginSearch(FText::FromString(SearchText),Case,false);
	//
	return FReply::Handled();
}

FReply SCS_CodeEditorCore::OnClickedReplaceSearch() {
	if (!SCRIPT_EDITOR.IsValid()) {return FReply::Unhandled();}
	if (ReplaceText.IsEmpty()) {return FReply::Handled();}
	if (SearchText.IsEmpty()) {return FReply::Handled();}
	if (!HasScript()) {return FReply::Unhandled();}
	//
	//
	FText DO = FText::FromString(FString(ScriptSource->GetSource()).Replace(*SearchText,*ReplaceText));
	SCRIPT_EDITOR->BeginEditTransaction();
	 SCRIPT_EDITOR->SetText(DO);
	SCRIPT_EDITOR->EndEditTransaction();
	//
	return FReply::Handled();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SCS_CodeEditorCore::OnScriptHorizontalScroll(float Offset) {}
void SCS_CodeEditorCore::OnInternalVerticalScroll(float Offset) {
	VS_SCROLL_BOX->SetScrollOffset(Offset);
}

void SCS_CodeEditorCore::OnVerticalScroll(float Offset) {
	VS_SCROLL->SetState(VS_SCROLL_BOX->GetScrollOffset(),VS_SCROLL_BOX->GetViewOffsetFraction());
}

void SCS_CodeEditorCore::OnScriptTextChanged(const FText &InText, ETextCommit::Type CommitType) {
	if (IsSourceLocked()) {return;}
	//
	SetScriptText(InText);
	SetLineCountList(GetLineCount());
}

void SCS_CodeEditorCore::OnScriptTextComitted(const FText &NewText, ETextCommit::Type CommitInfo) {
	if (IsSourceLocked()) {return;}
	//
	SetScriptText(NewText);
	SetLineCountList(GetLineCount());
}

void SCS_CodeEditorCore::OnReplaceTextComitted(const FText &NewText, ETextCommit::Type CommitInfo) {
	ReplaceText.Empty(); ReplaceText.Append(NewText.ToString());
}

void SCS_CodeEditorCore::OnSelectedLineCounterItem(TSharedPtr<FString>Item, ESelectInfo::Type SelectInfo) {
	if (!Item.IsValid()) {return;}
	//
	const int32 LineID = FCString::Atoi(**Item.Get());
	LINE_COUNTER->SetItemSelection(Item,false);
	//
	FSlateApplication::Get().SetKeyboardFocus(SCRIPT_EDITOR.ToSharedRef());
	SCRIPT_EDITOR->GoToLineColumn(LineID-1,0);
	SCRIPT_EDITOR->SelectLine();
}

TSharedRef<ITableRow> SCS_CodeEditorCore::OnGenerateLineCounter(TSharedPtr<FString>Item, const TSharedRef<STableViewBase>&OwnerTable) {
	TSharedRef<SImage>ICON = SNew(SImage).Image(FKCS_NodeStyle::Get()->GetBrush(TEXT("KCS.Success")));
	//
	if (ErrorType==EMonoCompilerResult::Error) {ICON=SNew(SImage).Image(FKCS_NodeStyle::Get()->GetBrush(TEXT("KCS.Error")));}
	if (ErrorType==EMonoCompilerResult::Warning) {ICON=SNew(SImage).Image(FKCS_NodeStyle::Get()->GetBrush(TEXT("KCS.Warning")));}
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

void SCS_CodeEditorCore::OnInvokedSearch(bool DoSearch) {
	ReplaceText = TEXT("Replace..");
	ViewSearchBox = DoSearch;
	//
	if (ViewSearchBox) {
		FSlateApplication::Get().SetKeyboardFocus(SEARCH_TEXT.ToSharedRef());
	}///
}

void SCS_CodeEditorCore::OnSearchSensitiveChanged(ECheckBoxState NewState) {
	SensitiveSearch = (NewState==ECheckBoxState::Checked);
}

void SCS_CodeEditorCore::OnSearchTextChanged(const FText &InText, ETextCommit::Type CommitType) {
	SearchText = InText.ToString();
}

void SCS_CodeEditorCore::OnSearchTextComitted(const FText &NewText, ETextCommit::Type CommitInfo) {
	SearchText.Empty(); SearchText.Append(NewText.ToString());
	//
	const ESearchCase::Type Case = (SensitiveSearch) ? ESearchCase::CaseSensitive : ESearchCase::IgnoreCase;
	SCRIPT_EDITOR->BeginSearch(FText::FromString(SearchText),Case,false);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SCS_CodeEditorCore::GoToTextLocation(const FTextLocation &Location) {
	if (SCRIPT_EDITOR.IsValid()) {
		FSlateApplication::Get().SetKeyboardFocus(SCRIPT_EDITOR.ToSharedRef());
		//
		if (Location.IsValid()) {
			SCRIPT_EDITOR->GoToLineColumn(Location.GetLineIndex(),Location.GetOffset());
			SCRIPT_EDITOR->SelectLine();
		}///
	}///
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SCS_CodeEditorCore::UpdateTextEditorScriptReference() {
	if (!HasScript()) {return;}
	//
	if (SCRIPT_EDITOR.IsValid()) {
		SCRIPT_EDITOR->SetScriptSource(ScriptSource);
	}///
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#undef LOCTEXT_NAMESPACE

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////