//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
///			Copyright 2021 (C) Bruno Xavier B. Leite
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "KCS_TextEditorWidget.h"

#include "KCS_TextSyntaxHighlighter.h"
#include "KCS_MonoAnalyzer.h"
#include "KCS_NodeStyle.h"

#include "MagicNodeSharp.h"

#include "Widgets/Text/SlateEditableTextLayout.h"
#include "Editor/EditorStyle/Public/EditorStyle.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SKCS_TextEditorWidget::Construct(const FArguments &InArgs) {
	SuggestFocusID = INDEX_NONE;
	SuggestPicked = INDEX_NONE;
	SuggestRootID = 0;
	//
	VScroll = InArgs._VScrollBar;
	Marshall = InArgs._Marshaller;
	//
	OnInvokedSearch = InArgs._OnInvokeSearch;
	KeyboardFocus = InArgs._CanKeyboardFocus.Get();
	//
	SMultiLineEditableText::Construct(
		SMultiLineEditableText::FArguments()
		.TextStyle(&FKCS_NodeStyle::Get().Get()->GetWidgetStyle<FTextBlockStyle>("KCS.CodeBlockStyle"))
		.Font(FKCS_NodeStyle::Get().Get()->GetWidgetStyle<FTextBlockStyle>("KCS.CodeBlockStyle").Font)
		.OnCursorMoved(this,&SKCS_TextEditorWidget::OnTextCursorMoved)
		.OnTextCommitted(InArgs._OnTextCommitted)
		.OnTextChanged(InArgs._OnTextChanged)
		.HScrollBar(InArgs._HScrollBar)
		.VScrollBar(InArgs._VScrollBar)
		.Marshaller(InArgs._Marshaller)
		.IsReadOnly(InArgs._IsReadOnly)
		.AllowContextMenu(true)
		.AutoWrapText(false)
		.Text(InArgs._Text)
		.Margin(0.f)
	);//
	//
	FontMeasure = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
	TypeWidth = FontMeasure->Measure("A",FKCS_NodeStyle::Get().Get()->GetWidgetStyle<FTextBlockStyle>("KCS.CodeBlockStyle").Font).X;
	LineHeight = FontMeasure->Measure("A",FKCS_NodeStyle::Get().Get()->GetWidgetStyle<FTextBlockStyle>("KCS.CodeBlockStyle").Font).Y;
	//
	LineCount = CountLines();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SKCS_TextEditorWidget::Tick(const FGeometry &AllottedGeometry, const double CurrentTime, const float DeltaTime) {
	EditableTextLayout->Tick(AllottedGeometry,CurrentTime,DeltaTime);
}

int32 SKCS_TextEditorWidget::OnPaint(const FPaintArgs &Args, const FGeometry &Geometry, const FSlateRect &CullingRect, FSlateWindowElementList &OutDrawElements, int32 LayerID, const FWidgetStyle &WidgetStyle, bool ParentEnabled) const {
	const ESlateDrawEffect DrawEffects = ESlateDrawEffect::None;
	const float AllotedHeight = Geometry.GetLocalSize().Y;
	const float AllotedWidth = Geometry.GetLocalSize().X;
	//
	//
	if (!bIsReadOnly.Get()) {
		for (int32 L=0; L<=(LineCount); L++) {
			FSlateDrawElement::MakeBox(
				OutDrawElements, LayerID,
				Geometry.ToPaintGeometry(FVector2D(0,(LineHeight*L)-EditableTextLayout->GetScrollOffset().Y),FVector2D(AllotedWidth,LineHeight)),
				FKCS_NodeStyle::Get().Get()->GetBrush("KCS.Lines"), DrawEffects, GetLineIndexColor(L)
			);//
		}///
		//
		LayerID++;
	}///
	//
	//
	LayerID = SMultiLineEditableText::OnPaint(Args,Geometry,CullingRect,OutDrawElements,LayerID,WidgetStyle,ParentEnabled); LayerID++;
	//
	//
	if (!bIsReadOnly.Get()) {
		FSlateDrawElement::MakeBox(
			OutDrawElements, LayerID,
			Geometry.ToPaintGeometry(FVector2D(0,(LineHeight*CursorLocation.GetLineIndex())-EditableTextLayout->GetScrollOffset().Y),FVector2D(AllotedWidth,LineHeight)),
			FKCS_NodeStyle::Get().Get()->GetBrush("KCS.Focus"), DrawEffects, FLinearColor(0.1f,0.5f,1.f,0.45f)
		);//
		//
		LayerID++;
		//
		if (HasSuggestion()&&(CompletionBoxSize.X)) {
			FVector2D BoxSize = GetCompletionBoxSize();
			FVector2D BoxPos = GetCompletionBoxPos();
			//
			FVector2D BorderPos = FVector2D(BoxPos.X-4,BoxPos.Y-4);
			FVector2D BorderSize = FVector2D(BoxSize.X+8,BoxSize.Y+8);
			//
			FSlateDrawElement::MakeBox(
				OutDrawElements, LayerID,
				Geometry.ToPaintGeometry(BorderPos,BorderSize),
				FEditorStyle::GetBrush("Graph.Node.Body"),
				DrawEffects, FLinearColor::White
			);//
			//
			FSlateDrawElement::MakeBox(
				OutDrawElements, LayerID,
				Geometry.ToPaintGeometry(BoxPos,BoxSize),
				FEditorStyle::GetBrush("Menu.Background"),
				DrawEffects, FLinearColor(0.025f,0.045f,0.065f,0.695f)
			);//
			//
			for (int32 I=SuggestRootID; I<(SuggestRootID+MAX_SUGGESTION_ROWS); ++I) {
				if (SuggestionResults.IsValidIndex(I)) {
					const float LineFraction = LineHeight/1.5f;
					//
					FVector2D TextPos = BoxPos;
					TextPos.Y += LineHeight * (I-SuggestRootID);
					FVector2D Position = FVector2D(TextPos.X+(LineFraction/4),TextPos.Y+(LineFraction/4));
					//
					FSlateDrawElement::MakeBox(
						OutDrawElements, LayerID,
						Geometry.ToPaintGeometry(Position,FVector2D(LineFraction,LineFraction)),
						GetSuggestionIcon(SuggestionResults[I]),
						DrawEffects, FLinearColor::White
					);//
					//
					TextPos.X += (LineHeight*1.5f);
					//
					FSlateDrawElement::MakeText(
						OutDrawElements, LayerID,
						Geometry.ToPaintGeometry(TextPos,FVector2D(BoxSize.X,LineHeight)),
						SuggestionResults[I], FCoreStyle::GetDefaultFontStyle("Bold",12),
						ESlateDrawEffect::NoBlending, GetSuggestionColor(SuggestionResults[I])
					);//
				}///
			}///
			//
			BoxPos.Y += (LineHeight*SuggestFocusID);
			//
			if (SuggestFocusID>INDEX_NONE) {
				FSlateDrawElement::MakeBox(
					OutDrawElements, LayerID,
					Geometry.ToPaintGeometry(BoxPos,FVector2D(BoxSize.X,LineHeight)),
					FKCS_NodeStyle::Get().Get()->GetBrush("KCS.Lines"),
					DrawEffects, FLinearColor(0.5f,0.5f,1.f,0.45f)
				);//
			}///
		}///
		//
		if ((KeywordInfo.Len()>0)&&(CompletionBoxSize.X)) {
			FVector2D BoxSize = GetCompletionBoxSize();
			FVector2D BoxPos = GetCompletionBoxPos();
			//
			FVector2D FontSize = FontMeasure->Measure(KeywordInfo,FCoreStyle::GetDefaultFontStyle("Default",11));
			FVector2D BoxOffset = FVector2D(BoxPos.X+BoxSize.X+6,BoxPos.Y); FontSize.X *= 1.2f;
			FVector2D BorderSize = FVector2D(FontSize.X+4,FontSize.Y+4);
			//
			FSlateDrawElement::MakeBox(
				OutDrawElements, LayerID,
				Geometry.ToPaintGeometry(FVector2D(BoxOffset.X,((CursorLocation.GetLineIndex()*LineHeight)-EditableTextLayout->GetScrollOffset().Y)+(LineHeight*(SuggestFocusID+1))),BorderSize),
				FEditorStyle::GetBrush("Graph.Node.Body"), DrawEffects, FLinearColor(0.15f,0.25f,0.25f,0.95f)
			);//
			//
			FSlateDrawElement::MakeText(
				OutDrawElements, LayerID,
				Geometry.ToPaintGeometry(FVector2D(BoxOffset.X+4,((CursorLocation.GetLineIndex()*LineHeight)-EditableTextLayout->GetScrollOffset().Y)+(LineHeight*(SuggestFocusID+1.1f))),FontSize),
				KeywordInfo, FCoreStyle::GetDefaultFontStyle("Default",11), ESlateDrawEffect::NoBlending, FLinearColor::White
			);//
		}///
	}///
	//
	//
	return LayerID;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SKCS_TextEditorWidget::OnTextCursorMoved(const FTextLocation &NewPosition) {
	auto WidgetWindow = FSlateApplication::Get().FindWidgetWindow(this->AsShared());
	const float DPIScale = (WidgetWindow.IsValid()) ? WidgetWindow->GetNativeWindow()->GetDPIScaleFactor() : 1.f;
	//
	LastPosition = CursorLocation;
	CursorLocation = NewPosition;
	//
	int32 Tabs = (CountTabs()*(FMath::TruncToInt(TypeWidth)*3));
	//
	CompletionBoxPos = FVector2D(
		(Tabs)+(FMath::TruncToInt(TypeWidth)*CursorLocation.GetOffset()),
		((FMath::TruncToInt(LineHeight)*CursorLocation.GetLineIndex())-EditableTextLayout->GetScrollOffset().Y)+(LineHeight+4)
	); CompletionBoxPos.X /= DPIScale;
	//
	if (HasSuggestion()&&(AutoKeyword.Len()>0)) {
		if (CursorLocation.GetLineIndex()!=LastPosition.GetLineIndex()) {
			ClearSuggestion();
		}///
	}///
	//
	OnMovedCursor.ExecuteIfBound(NewPosition);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SKCS_TextEditorWidget::GoToLineColumn(int32 Line, int32 Column) {
	FTextLocation Location(Line,Column);
	//
	ScrollTo(Location);
	GoTo(Location);
}

void SKCS_TextEditorWidget::SelectLine() {
	EditableTextLayout->JumpTo(ETextLocation::EndOfLine,ECursorAction::MoveCursor);
	EditableTextLayout->JumpTo(ETextLocation::BeginningOfLine,ECursorAction::SelectText);
}

void SKCS_TextEditorWidget::DeleteSelectedText() {
	EditableTextLayout->DeleteSelectedText();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const FString & SKCS_TextEditorWidget::CaptureCursor() const {
	if (EditableTextLayout.IsValid()) {
		auto Run = EditableTextLayout->GetRunUnderCursor();
		//
		UnderCursor.Empty();
		if (Run.IsValid() && CursorLocation.IsValid()) {
			Run->AppendTextTo(UnderCursor);
		}///
	}///
	//
	return UnderCursor;
}

const FString & SKCS_TextEditorWidget::GetUnderCursor() const {
	return UnderCursor;
}

const FVector2D SKCS_TextEditorWidget::GetScrollOffset() const {
	return EditableTextLayout->GetScrollOffset();
}

const FTextLocation & SKCS_TextEditorWidget::GetCursorLocation() const {
	return CursorLocation;
}

const FString & SKCS_TextEditorWidget::GetCurrentLineAtCursor() const {
	return CurrentLine;
}

const FString & SKCS_TextEditorWidget::GetAutoCompleteSubject() const {
	return AutoKeyword;
}

const FString & SKCS_TextEditorWidget::GetKeywordInfo() const {
	return KeywordInfo;
}

const int32 SKCS_TextEditorWidget::GetLineCount() const {
	return LineCount;
}

int32 SKCS_TextEditorWidget::CountLines() const {
	int32 Count = 0;
	//
	for (const TCHAR &CH : GetPlainText().ToString().GetCharArray()) {
		if (CH==NLC) {Count++;}
	}///
	//
	return Count;
}

int32 SKCS_TextEditorWidget::CountSelectedLines() const {
	if (!AnyTextSelected()) {return 0;}
	//
	const auto &Selected = GetSelectedText().ToString();
	int32 Count = 1;
	//
	for (const TCHAR &CH : Selected.GetCharArray()) {
		if (CH==NLC) {Count++;}
	}///
	//
	return Count;
}

int32 SKCS_TextEditorWidget::CountTabs(bool BreakOnAlpha) const {
	int32 Count = 0;
	int32 Index = 0;
	//
	for (const TCHAR &CH : CurrentLine.GetCharArray()) {
		if (CH==NLT) {Count++;} else if (BreakOnAlpha) {
			if (!TChar<WIDECHAR>::IsWhitespace(CH)) {break;}
		} Index++;
		//
		if (Index>=CursorLocation.GetOffset()) {break;}
	}///
	//
	return Count;
}

int32 SKCS_TextEditorWidget::CountChars(const TCHAR &CH, const FString &Word) const {
	int32 Count = 0;
	//
	for (const TCHAR &CC : Word.GetCharArray()) {
		if (CC==CH) {Count++;}
	}///
	//
	return Count;
}

FVector2D SKCS_TextEditorWidget::CalculateWordSize() const {
	if (UnderCursor.IsEmpty()) {return FVector2D::ZeroVector;}
	//
	return FontMeasure->Measure(UnderCursor,FKCS_NodeStyle::Get().Get()->GetWidgetStyle<FTextBlockStyle>("KCS.CodeBlockStyle").Font);
}

const FLinearColor SKCS_TextEditorWidget::GetLineIndexColor(int32 Line) const {
	if (ErrorLine==Line) {
		if (ErrorType==EMonoCompilerResult::Error) {
			return FLinearColor(1.f,0.f,0.f,0.25f);
		} else if (ErrorType==EMonoCompilerResult::Warning) {
			return FLinearColor(1.f,1.f,0.f,0.25f);
		}///
	}///
	//
	if (Line % 2 == 0) {
		return FLinearColor(0.1f,1.f,0.5f,0.01f);
	}///
	//
	return FLinearColor(0.1f,0.5f,1.f,0.05f);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

FReply SKCS_TextEditorWidget::OnKeyChar(const FGeometry &Geometry, const FCharacterEvent &CharacterEvent) {
	const TCHAR CH = CharacterEvent.GetCharacter();
	//
	if (CH==TEXT('\n')||CH==TEXT('\r')) {
		EditableTextLayout->BeginEditTransation();
		//
		FString Carr;
		for (int32 I=0; I<LastLineFeed; ++I) {Carr.AppendChar(NLT);}
		InsertTextAtCursor(Carr); LastLineFeed = 0; Carr.Empty();
		//
		if (NextLineFeed > 0) {
			FTextLocation LOC = CursorLocation; InsertTextAtCursor(NLS);
			for (int32 I=0; I<NextLineFeed; ++I) {Carr.AppendChar(NLT);}
			InsertTextAtCursor(Carr); NextLineFeed = 0; Carr.Empty();
			//
			GoTo(LOC);
		} else {
			int32 AtLine = CursorLocation.GetLineIndex();
			int32 Offset = CursorLocation.GetOffset();
			//
			TCHAR Next = (EditableTextLayout->IsAtEndOfLine(CursorLocation)) ? NLC : EditableTextLayout->GetCharacterAt(FTextLocation(AtLine,Offset));
			//
			if (IsCloseBracket(Next)) {
				FTextLocation LOC = CursorLocation;
				InsertTextAtCursor(NLS); GoTo(LOC);
			}///
		}///
		//
		EditableTextLayout->EndEditTransaction();
		//
		return FReply::Handled();
	}///
	//
	if (TChar<WIDECHAR>::IsWhitespace(CH)) {
		if (!IsTextReadOnly()) {
			if (CharacterEvent.IsCommandDown()||CharacterEvent.IsControlDown()) {
				return FReply::Handled();
			} else {
				SMultiLineEditableText::OnKeyChar(Geometry,CharacterEvent);
			} return FReply::Handled();
		} else {return FReply::Unhandled();}
	} else if (CH==TEXT('\t')) {
		return FReply::Handled();
	}///
	//
	return SMultiLineEditableText::OnKeyChar(Geometry,CharacterEvent);
}

FReply SKCS_TextEditorWidget::OnKeyDown(const FGeometry &Geometry, const FKeyEvent &KeyEvent) {
	if (IsTextReadOnly()) {return FReply::Unhandled();}
	//
	FKey Key = KeyEvent.GetKey();
	//
	if (Key==EKeys::Tab) {
		ClearSuggestion();
		//
		FString In{};
		uint32 SelectEnd = 0;
		uint32 SelectStart = 0;
		In.AppendChar(TEXT('\t'));
		ITextInputMethodContext::ECaretPosition Caret = ITextInputMethodContext::ECaretPosition::Beginning;
		//
		if (AnyTextSelected()) {
			auto Context = EditableTextLayout->GetTextInputMethodContext();
			Context->GetSelectionRange(SelectStart,SelectEnd,Caret);
			//
			FTextLocation Origin = CursorLocation;
			int32 Lines = CountSelectedLines();
			//
			EditableTextLayout->BeginEditTransation();
			//
			if (Caret==ITextInputMethodContext::ECaretPosition::Beginning) {
				EditableTextLayout->ClearSelection();
				InsertTextAtCursor(In);
				//
				for (int32 I=1; I<Lines; ++I) {
					FTextLocation Local = FTextLocation(Origin.GetLineIndex()+I,0);
					EditableTextLayout->GoTo(Local); InsertTextAtCursor(In);
				}///
				//
				EditableTextLayout->GoTo(FTextLocation(Origin.GetLineIndex(),Origin.GetOffset()+1));
				Context->SetSelectionRange(SelectStart+1,SelectEnd+(Lines-1),ITextInputMethodContext::ECaretPosition::Beginning);
			} else {
				TOptional<SlateEditableTextTypes::ECursorAlignment>Left = SlateEditableTextTypes::ECursorAlignment::Left;
				FTextLocation Jump; EditableTextLayout->TranslateLocationVertical(Origin,-(Lines-1),Geometry.Scale,Jump,Left);
				Jump = FTextLocation(Jump.GetLineIndex(),0);
				EditableTextLayout->GoTo(Jump);
				InsertTextAtCursor(In);
				//
				for (int32 I=1; I<Lines; ++I) {
					FTextLocation Local = FTextLocation(Jump.GetLineIndex()+I,0);
					EditableTextLayout->GoTo(Local); InsertTextAtCursor(In);
				}///
				//
				EditableTextLayout->GoTo(FTextLocation(Origin.GetLineIndex(),Origin.GetOffset()+1));
				Context->SetSelectionRange(SelectStart+1,SelectEnd+(Lines-1),ITextInputMethodContext::ECaretPosition::Beginning);
			}///
			//
			EditableTextLayout->EndEditTransaction();
		} else {
			EditableTextLayout->BeginEditTransation();
			//
			InsertTextAtCursor(In);
			//
			EditableTextLayout->EndEditTransaction();
		} EditableTextLayout->ClearSelection();
		//
		return FReply::Handled();
	}///
	//
	if ((Key==EKeys::Up)&&HasSuggestion()) {
		SuggestFocusID = FMath::Clamp((SuggestFocusID-1),-1,FMath::Min<int32>((SuggestionResults.Num()-1),(MAX_SUGGESTION_ROWS-1)));
		const bool CanScroll = ((SuggestRootID-1)>INDEX_NONE);
		//
		if (SuggestFocusID<(0)&&(CanScroll)) {
			SuggestFocusID = 0;
			SuggestRootID = FMath::Clamp((SuggestRootID-1),0,(SuggestionResults.Num()-1));
		} else if (SuggestFocusID<(0)) {SuggestFocusID=0;}
		//
		SuggestPicked = (SuggestRootID+SuggestFocusID);
		SearchKeywordInfo();
		//
		return FReply::Handled();
	}///
	//
	if ((Key==EKeys::Down)&&HasSuggestion()) {
		const int32 ROWS = FMath::Min<int32>((SuggestionResults.Num()-1),(MAX_SUGGESTION_ROWS-1));
		const bool CanScroll = ((SuggestRootID+1)<=((SuggestionResults.Num()-1)-(MAX_SUGGESTION_ROWS-1)));
		SuggestFocusID = FMath::Clamp((SuggestFocusID+1),0,FMath::Min<int32>((SuggestionResults.Num()-1),MAX_SUGGESTION_ROWS));
		//
		if (SuggestFocusID>(ROWS)&&(CanScroll)) {
			SuggestRootID = FMath::Clamp((SuggestRootID+1),0,(SuggestionResults.Num()-1));
			SuggestFocusID = FMath::Min<int32>((SuggestionResults.Num()-1),(MAX_SUGGESTION_ROWS-1));
		} else if (SuggestFocusID>(ROWS)) {SuggestFocusID=ROWS;}
		//
		SuggestPicked = (SuggestRootID+SuggestFocusID);
		SearchKeywordInfo();
		//
		return FReply::Handled();
	}///
	//
	if ((Key==EKeys::Enter)&&HasSuggestion()) {
		InsertPickedSuggestion();
		return FReply::Handled();
	} else if ((Key==EKeys::Enter)&&(KeyEvent.IsCommandDown()||KeyEvent.IsControlDown())) {
		FormatSourceCode(); return FReply::Handled();
	} else if (Key==EKeys::Enter) {
		EditableTextLayout->BeginEditTransation();
		//
		int32 AtLine = CursorLocation.GetLineIndex();
		int32 Offset = CursorLocation.GetOffset();
		//
		LastLineFeed = CountTabs(); NextLineFeed = LastLineFeed;
		TCHAR Last = (EditableTextLayout->IsAtBeginningOfLine(CursorLocation)) ? NLC : EditableTextLayout->GetCharacterAt(FTextLocation(AtLine,Offset-1));
		TCHAR Next = (EditableTextLayout->IsAtEndOfLine(CursorLocation)) ? NLC : EditableTextLayout->GetCharacterAt(FTextLocation(AtLine,Offset));
		//
		if (IsOpenBracket(Last)) {LastLineFeed++;}
		if (!IsCloseBracket(Next)) {NextLineFeed=0;}
		//
		SMultiLineEditableText::OnKeyDown(Geometry,KeyEvent);
		//
		EditableTextLayout->EndEditTransaction();
		EditableTextLayout->ClearSelection();
		//
		return FReply::Handled();
	}///
	//
	if ((Key==EKeys::Delete)&&(KeyEvent.IsShiftDown())) {
		ClearSuggestion();
		//
		EditableTextLayout->BeginEditTransation();
		//
		EditableTextLayout->JumpTo(ETextLocation::BeginningOfLine,ECursorAction::MoveCursor);
		EditableTextLayout->JumpTo(ETextLocation::EndOfLine,ECursorAction::SelectText);
		EditableTextLayout->CutSelectedTextToClipboard();
		//
		EditableTextLayout->EndEditTransaction();
		EditableTextLayout->ClearSelection();
		//
		return FReply::Handled();
	} else if (Key==EKeys::Delete) {
		ClearSuggestion();
		//
		EditableTextLayout->BeginEditTransation();
		//
		SMultiLineEditableText::OnKeyDown(Geometry,KeyEvent);
		//
		EditableTextLayout->EndEditTransaction();
		EditableTextLayout->ClearSelection();
		//
		return FReply::Handled();
	}///
	//
	if ((Key==EKeys::Escape||Key==EKeys::BackSpace||Key==EKeys::SpaceBar)&&(HasSuggestion())) {
		if (!(KeyEvent.IsControlDown()||KeyEvent.IsCommandDown())) {
			OnInvokedSearch.ExecuteIfBound(false);
			ClearSuggestion();
			//
			return FReply::Handled();
		}///
	}///
	//
	if ((Key==EKeys::F)&&(KeyEvent.IsControlDown()||KeyEvent.IsCommandDown())) {
		ShowSearchBox = !ShowSearchBox;
		//
		OnInvokedSearch.ExecuteIfBound(ShowSearchBox);
		//
		return FReply::Handled();
	}///
	//
	return SMultiLineEditableText::OnKeyDown(Geometry,KeyEvent);
}

FReply SKCS_TextEditorWidget::OnKeyUp(const FGeometry &Geometry, const FKeyEvent &KeyEvent) {
	if (IsTextReadOnly()) {return FReply::Unhandled();}
	//
	LineCount = CountLines();
	FKey Key = KeyEvent.GetKey();
	//
	if (HasSuggestion()) {
		if ((Key==EKeys::LeftControl)||(Key==EKeys::RightControl)||(Key==EKeys::LeftCommand)||(Key==EKeys::RightCommand)) {return SMultiLineEditableText::OnKeyUp(Geometry,KeyEvent);}
		if ((Key==EKeys::Up)||(Key==EKeys::Down)||(Key==EKeys::SpaceBar)) {return SMultiLineEditableText::OnKeyUp(Geometry,KeyEvent);}
		if ((Key==EKeys::LeftShift)||(Key==EKeys::RightShift)) {return SMultiLineEditableText::OnKeyUp(Geometry,KeyEvent);}
		if ((Key==EKeys::LeftAlt)||(Key==EKeys::RightAlt)) {return SMultiLineEditableText::OnKeyUp(Geometry,KeyEvent);}
		//
		if (TChar<WIDECHAR>::IsAlpha(KeyEvent.GetCharacter())) {FilterSuggestion(); return FReply::Handled();}
		//
		if (!TChar<WIDECHAR>::IsAlpha(KeyEvent.GetCharacter())) {
			ClearSuggestion();
			//
			if (Key==EKeys::Period) {AutoSuggest();}
			//
			return FReply::Handled();
		}///
	} else if ((Key==EKeys::SpaceBar)&&(KeyEvent.IsControlDown()||KeyEvent.IsCommandDown())) {
		AutoSuggest(); return FReply::Handled();
	} else if ((Key==EKeys::C||Key==EKeys::V)&&(KeyEvent.IsControlDown()||KeyEvent.IsCommandDown())) {
		return SMultiLineEditableText::OnKeyUp(Geometry,KeyEvent);
	} else if (!HasSuggestion()) {
		if (Key==EKeys::Period) {AutoSuggest(); return FReply::Handled();} else
		if (TChar<WIDECHAR>::IsAlpha(KeyEvent.GetCharacter())) {SuggestKeyword();}
	}///
	//
	return SMultiLineEditableText::OnKeyUp(Geometry,KeyEvent);
}

FReply SKCS_TextEditorWidget::OnMouseButtonDown(const FGeometry &Geometry, const FPointerEvent &MouseEvent) {
	if (IsTextReadOnly()) {return FReply::Unhandled();}
	//
	EditableTextLayout->GetCurrentTextLine(CurrentLine);
	CaptureCursor();
	//
	if (!HasSuggestion()&&(MouseEvent.GetEffectingButton()==EKeys::LeftMouseButton)&&(MouseEvent.IsControlDown()||MouseEvent.IsCommandDown())) {
		EditableTextLayout->SelectWordAt(Geometry,MouseEvent.GetScreenSpacePosition());
		UnderCursor = GetSelectedText().ToString();
		//
		const FString IURL = FString::Printf(TEXT("https://docs.microsoft.com/en-us/search/?scope=.NET&terms=%s"),*UnderCursor);
		UKismetSystemLibrary::LaunchURL(IURL);
		//
		return FReply::Handled();
	}///
	//
	return SMultiLineEditableText::OnMouseButtonDown(Geometry,MouseEvent);
}

FReply SKCS_TextEditorWidget::OnMouseButtonUp(const FGeometry &Geometry, const FPointerEvent &MouseEvent) {
	if (IsTextReadOnly()) {return FReply::Unhandled();}
	//
	EditableTextLayout->GetCurrentTextLine(CurrentLine);
	CaptureCursor();
	//
	if (HasSuggestion()) {
		if (CursorLocation.GetLineIndex()!=LastPosition.GetLineIndex()) {
			ClearSuggestion(); return FReply::Handled();
		}///
	}///
	//
	return SMultiLineEditableText::OnMouseButtonUp(Geometry,MouseEvent);
}

FReply SKCS_TextEditorWidget::OnMouseMove(const FGeometry &Geometry, const FPointerEvent &MouseEvent) {
	MousePosition = Geometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());
	//
	return SMultiLineEditableText::OnMouseMove(Geometry,MouseEvent);
}

FReply SKCS_TextEditorWidget::OnMouseWheel(const FGeometry &Geometry, const FPointerEvent &MouseEvent) {
	if (HasSuggestion()&&(SuggestionResults.Num()>MAX_SUGGESTION_ROWS)) {
		const bool CanScroll = ((SuggestRootID+1)<=((SuggestionResults.Num()-1)-(MAX_SUGGESTION_ROWS-1)));
		//
		if (MouseEvent.GetWheelDelta()>0) {
			SuggestRootID = FMath::Clamp((SuggestRootID-1),0,(SuggestionResults.Num()-1));
		} else if (CanScroll) {
			SuggestRootID = FMath::Clamp((SuggestRootID+1),0,(SuggestionResults.Num()-1));
		}///
		//
		SuggestFocusID = FMath::Clamp(SuggestFocusID,0,FMath::Min<int32>((SuggestionResults.Num()-1),MAX_SUGGESTION_ROWS));
		SuggestPicked = (SuggestRootID+SuggestFocusID);
		SearchKeywordInfo();
		//
		return FReply::Handled();
	}///
	//
	return (SMultiLineEditableText::OnMouseWheel(Geometry,MouseEvent));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

FVector2D SKCS_TextEditorWidget::GetCompletionBoxPos() const {
	return CompletionBoxPos;
}

FVector2D SKCS_TextEditorWidget::GetCompletionBoxSize() const {
	return CompletionBoxSize;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const bool SKCS_TextEditorWidget::IsOperator(const TCHAR &CH) const {
	return (
		CH==TEXT('(') ||
		CH==TEXT(')') ||
		CH==TEXT('[') ||
		CH==TEXT(']') ||
		CH==TEXT('{') ||
		CH==TEXT('}') ||
		CH==TEXT('<') ||
		CH==TEXT('>') ||
		CH==TEXT(':') ||
		CH==TEXT('.') ||
		CH==TEXT(',') ||
		CH==TEXT(';') ||
		CH==TEXT('-') ||
		CH==TEXT('+') ||
		CH==TEXT('=') ||
		CH==TEXT('&') ||
		CH==TEXT('/') ||
		CH==TEXT('?') ||
		CH==TEXT('!') ||
		CH==TEXT('*') ||
		CH==TEXT('%') ||
		CH==TEXT('#') ||
		CH==TEXT('|') ||
		CH==TEXT('\\')
	);//
}

const bool SKCS_TextEditorWidget::IsTab(const TCHAR &CH) const {
	return (CH==TEXT('\t'));
}

const bool SKCS_TextEditorWidget::IsBracket(const TCHAR &CH) const {
	return (
		IsOpenBracket(CH) || IsCloseBracket(CH)
	);//
}

const bool SKCS_TextEditorWidget::IsAngular(const TCHAR &CH) const {
	return (
		CH==TEXT('<') ||
		CH==TEXT('>')
	);//
}

const bool SKCS_TextEditorWidget::IsParentheses(const TCHAR &CH) const {
	return (
		CH==TEXT('(') ||
		CH==TEXT(')')
	);//
}

const bool SKCS_TextEditorWidget::IsOpenBracket(const TCHAR &CH) const {
	return (
		CH==TEXT('(') ||
		CH==TEXT('[') ||
		CH==TEXT('{') ||
		CH==TEXT('<')
	);//
}

const bool SKCS_TextEditorWidget::IsCloseBracket(const TCHAR &CH) const {
	return (
		CH==TEXT(')') ||
		CH==TEXT(']') ||
		CH==TEXT('}') ||
		CH==TEXT('>')
	);//
}

const bool SKCS_TextEditorWidget::HasSuggestion() const {
	return (SuggestionResults.Num()>0);
}

const bool SKCS_TextEditorWidget::IsAutoComplete(const FString &Keyword) const {
	return (
		(Keyword.EndsWith(TEXT("."))||Keyword.EndsWith(TEXT("@"))) &&
		(!IsAutoCompleteTree(Keyword))
	);///
}

const bool SKCS_TextEditorWidget::IsAutoCompleteTree(const FString &Keyword) const {
	int32 Count = 0;
	//
	for (const TCHAR &CH : Keyword.GetCharArray()) {
		if (CH==TEXT('.')) {Count++;}
	}///
	//
	return (
		((Keyword.EndsWith(TEXT("."))||Keyword.EndsWith(TEXT("@")))&&(Count>1)) ||
		(!Keyword.EndsWith(TEXT("."))&&(Count>0))
	);///
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const FString SKCS_TextEditorWidget::FilterKeyword(const FString &Keyword) const {
	FString Subject = Keyword; FString Raw{};
	//
	bool IsMethodCall = false;
	int32 Open = CountChars(TEXT(')'),Subject);
	//
	if (Open>0) {
		int32 Range = Subject.Len()-1;
		//
		for (int32 R=Range; R>=0; --R) {
			if (Subject[R]==TEXT('(')){--Open; continue;}
			if (Open<1) {Raw.AppendChar(Subject[R]); IsMethodCall=true;}
		}///
		//
		Raw.ReverseString();
		Subject = Raw.TrimStartAndEnd();
	} else if (Subject.Contains(TEXT("("))) {
		Subject.Split(TEXT("("),nullptr,&Subject,ESearchCase::IgnoreCase,ESearchDir::FromEnd);
	}///
	//
	Open = CountChars(TEXT('>'),Subject);
	//
	if (Open>0) {
		Raw.Empty();
		int32 Range = Subject.Len()-1;
		//
		for (int32 R=Range; R>=0; --R) {
			if (Subject[R]==TEXT('<')){--Open; continue;}
			if (Open<1) {Raw.AppendChar(Subject[R]);}
		}///
		//
		Raw.ReverseString();
		Subject = Raw.TrimStartAndEnd();
	} else if (Subject.Contains(TEXT("<"))) {
		Subject.Split(TEXT("<"),&Subject,nullptr,ESearchCase::IgnoreCase,ESearchDir::FromEnd);
	}///
	//
	if (IsMethodCall) {Subject.AppendChar(TEXT('@'));}
	//
	return Subject;
}

const FString SKCS_TextEditorWidget::ParseAutoCompleteWord() const {
	EditableTextLayout->GetCurrentTextLine(CurrentLine);
	CaptureCursor();
	//
	FString ParseLine{};
	if (CursorLocation.GetOffset()<(CurrentLine.Len()-1)) {
		ParseLine = CurrentLine.Mid(0,CursorLocation.GetOffset());
	} else {ParseLine=CurrentLine;}
	//
	FString Subject = UnderCursor.TrimStartAndEnd();
	TCHAR OP = (Subject.Len()>0) ? Subject[Subject.Len()-1] : TEXT('\0');
	//
	if (Subject.IsEmpty()||TChar<WIDECHAR>::IsPunct(OP)) {
		FTextLocation Offset = CursorLocation;
		int32 Len = ParseLine.Len()-1;
		int32 I = Offset.GetOffset()-1;
		FString Raw{};
		//
		if (I>Len) {I=Len;}
		for (int32 L=I; L>=0; --L) {
			if (!ParseLine.IsValidIndex(L)) {--L;continue;}
			const TCHAR &CH = ParseLine[L];
			//
			if (IsOperator(CH)&&(!IsAngular(CH))&&(!IsParentheses(CH))&&(CH!=TEXT('.'))) {break;}
			if (TChar<WIDECHAR>::IsWhitespace(CH)) {break;}
			if (TChar<WIDECHAR>::IsLinebreak(CH)) {break;}
			if (IsTab(CH)) {break;}
			//
			Raw.AppendChar(CH);
		}///
		//
		Raw.ReverseString();
		Subject = Raw.TrimStartAndEnd();
		//
		if (IsAutoCompleteTree(Subject)) {
			TArray<FString>Tree = ParseAutoCompleteTree(Subject);
			Subject.Empty();
			//
			for (const auto &Sub : Tree) {
				Subject.Append(FilterKeyword(Sub)+TEXT("."));
			}///
			//
			if (Subject.EndsWith("@.")) {Subject.RemoveFromEnd(TEXT("."));}
		} else {
			Subject = FilterKeyword(Subject);
		}///
	}///
	//
	return Subject;
}

const TArray<FString>SKCS_TextEditorWidget::ParseAutoCompleteTree(const FString &Keyword) const {
	TArray<FString>Tree{};
	//
	if (!Keyword.Contains(TEXT("."))) {
		Tree.Add(Keyword);
	} else {
		Keyword.ParseIntoArray(Tree,TEXT("."));
	}///
	//
	return Tree;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SKCS_TextEditorWidget::SuggestKeyword() {
	if (CurrentLine.TrimStartAndEnd().StartsWith("//")||CurrentLine.TrimStartAndEnd().StartsWith("*")||CurrentLine.TrimStartAndEnd().StartsWith("/*")) {ClearSuggestion(); return;}
	if (ScriptSource.IsStale()||(!ScriptSource.IsValid())) {return;}
	//
	FString Keyword = ParseAutoCompleteWord();
	AutoKeyword = Keyword.TrimStartAndEnd();
	if (AutoKeyword.Len()<2) {return;}
	//
	IKCS_MonoAnalyzer::AutoSuggest(ScriptSource.Get(),AutoKeyword,SuggestionResults);
	//
	AutoSuggestCompleted();
}

void SKCS_TextEditorWidget::AutoSuggest() {
	if (CurrentLine.TrimStartAndEnd().StartsWith("//")||CurrentLine.TrimStartAndEnd().StartsWith("*")||CurrentLine.TrimStartAndEnd().StartsWith("/*")) {ClearSuggestion(); return;}
	if (ScriptSource.IsStale()||(!ScriptSource.IsValid())) {return;}
	//
	FString Keyword = ParseAutoCompleteWord();
	//
	if (Keyword.IsEmpty()) {
		ClearSuggestion();
	} else {
		DoAutoSuggestion(Keyword);
	}///
}

void SKCS_TextEditorWidget::DoAutoSuggestion(const FString &Keyword) {
	AutoKeyword = Keyword.TrimStartAndEnd();
	//
	if (IsAutoComplete(AutoKeyword)) {
		if (AutoKeyword.RemoveFromEnd(TEXT("."))) {
			IKCS_MonoAnalyzer::AutoSuggest(AutoKeyword,ScriptSource.Get(),SuggestionResults);
		} else if (AutoKeyword.EndsWith(TEXT("@"))) {
			IKCS_MonoAnalyzer::AutoSuggestMethod(AutoKeyword,ScriptSource.Get(),SuggestionResults);
		}///
		//
		AutoSuggestCompleted();
	} else if (IsAutoCompleteTree(AutoKeyword)) {
		TArray<FString>Tree = ParseAutoCompleteTree(AutoKeyword);
		if (Tree.Num()==0) {ClearSuggestion(); return;} else {
			if (AutoKeyword.EndsWith(TEXT("@"))) {
				IKCS_MonoAnalyzer::AutoSuggestMethod(Tree,ScriptSource.Get(),SuggestionResults);
			} else {
				IKCS_MonoAnalyzer::AutoSuggest(Tree,ScriptSource.Get(),SuggestionResults);
			} AutoSuggestCompleted();
		}///
	}///
}

void SKCS_TextEditorWidget::FilterSuggestion() {
	FString Keyword = ParseAutoCompleteWord();
	//
	if (IsAutoComplete(Keyword)) {
		Keyword.RemoveFromEnd(TEXT("."));
		TArray<FString>NewResults = SuggestionResults;
		//
		for (int32 I=SuggestionResults.Num()-1; I>=0; --I) {
			const FString &Item = SuggestionResults[I];
			//
			if (!Item.Contains(Keyword)) {NewResults.Remove(Item);}
			if (Keyword.Len()>Item.Len()) {NewResults.Remove(Item);}
		}///
		//
		if (NewResults.Num()>0) {
			NewResults.Shrink(); SuggestionResults = NewResults;
		} else if ((SuggestionResults.Num()==1)&&(Keyword.Len()>SuggestionResults[0].Len())) {
			ClearSuggestion(); KeywordInfo.Empty();
		}///
		//
		AutoSuggestCompleted();
	} else if (IsAutoCompleteTree(Keyword)) {
		const TArray<FString>Tree = ParseAutoCompleteTree(Keyword);
		//
		if (Tree.Num()==0) {return;} else {
			const FString &Key = Tree[Tree.Num()-1];
			TArray<FString>NewResults = SuggestionResults;
			//
			for (int32 I=SuggestionResults.Num()-1; I>=0; --I) {
				const FString &Item = SuggestionResults[I];
				//
				if (!Item.Contains(Key)) {NewResults.Remove(Item);}
				if (Key.Len()>Item.Len()) {NewResults.Remove(Item);}
			}///
			//
			if (NewResults.Num()>0) {
				NewResults.Shrink(); SuggestionResults = NewResults;
			} else if ((SuggestionResults.Num()==1)&&(Key.Len()>SuggestionResults[0].Len())) {
				ClearSuggestion(); KeywordInfo.Empty();
			}///
			//
			AutoSuggestCompleted();
		}///
	} else {
		TArray<FString>NewResults = SuggestionResults;
		for (int32 I=SuggestionResults.Num()-1; I>=0; --I) {
			const FString &Item = SuggestionResults[I];
			//
			if (!Item.Contains(Keyword)) {NewResults.Remove(Item);}
			if (Keyword.Len()>Item.Len()) {NewResults.Remove(Item);}
			if (Keyword.Equals(Item,ESearchCase::CaseSensitive)) {NewResults.Remove(Item);}
		}///
		//
		NewResults.Shrink();
		SuggestionResults = NewResults;
		//
		AutoSuggestCompleted();
	}///
}

void SKCS_TextEditorWidget::AutoSuggestCompleted() {
	if (HasSuggestion()) {
		CompletionBoxSize = FVector2D::ZeroVector;
		CompletionBoxSize.Y = FMath::Clamp(SuggestionResults.Num()*LineHeight,0.f,LineHeight*MAX_SUGGESTION_ROWS);
		//
		for (const auto &Suggestion : SuggestionResults) {
			const float Width = FontMeasure->Measure(Suggestion,FKCS_NodeStyle::Get().Get()->GetWidgetStyle<FTextBlockStyle>("KCS.CodeBlockStyle").Font).X+16;
			if (CompletionBoxSize.X<MIN_BOX_SIZE) {CompletionBoxSize.X=MIN_BOX_SIZE;}
			if (Width>CompletionBoxSize.X) {CompletionBoxSize.X=Width;}
		}///
		//
		if (SuggestPicked>SuggestionResults.Num()-1) {
			SuggestPicked = SuggestionResults.Num()-1;
		}///
	} else {CompletionBoxSize=FVector2D::ZeroVector;}
}

void SKCS_TextEditorWidget::InsertPickedSuggestion() {
	if (SuggestPicked>SuggestionResults.Num()-1) {
		SuggestPicked = SuggestionResults.Num()-1;
	}///
	//
	if (SuggestionResults.IsValidIndex(SuggestPicked)) {
		FString Insert = SuggestionResults[SuggestPicked];
		FString Keyword = ParseAutoCompleteWord();
		//
		if (IsAutoCompleteTree(Keyword)) {
			const TArray<FString>Tree = ParseAutoCompleteTree(Keyword);
			const FString &Key = Tree[Tree.Num()-1];
			//
			if ((Insert.Len()>Key.Len())&&(!Tree[0].Equals(Key,ESearchCase::CaseSensitive))) {Insert.RemoveFromStart(Key);}
			//
			EditableTextLayout->BeginEditTransation();
			{
				EditableTextLayout->DeleteSelectedText();
				if (!Insert.Equals(Key)) {InsertTextAtCursor(Insert);}
				//if (IKCS_MonoAnalyzer::GetMethodInfo(Tree).IsValid()) {InsertTextAtCursor(TEXT("()"));}
			}
			EditableTextLayout->EndEditTransaction();
		} else {
			Insert.RemoveFromStart(Keyword);
			//const auto Method = IKCS_MonoAnalyzer::GetMethodInfo(Insert);
			//
			EditableTextLayout->BeginEditTransation();
			{
				EditableTextLayout->DeleteSelectedText();
				if (!Insert.Equals(Keyword)) {InsertTextAtCursor(Insert);}
				//if (Method.IsValid()) {InsertTextAtCursor(TEXT("()"));}
			}
			EditableTextLayout->EndEditTransaction();
		}///
	}///
	//
	ClearSuggestion();
	CountLines();
}

void SKCS_TextEditorWidget::ClearSuggestion() {
	CompletionBoxSize = FVector2D::ZeroVector;
	//
	SuggestionResults.Empty();
	AutoKeyword.Empty();
	KeywordInfo.Empty();
	//
	SuggestRootID = 0;
	SuggestPicked = INDEX_NONE;
	SuggestFocusID = INDEX_NONE;
	//
	IKCS_MonoAnalyzer::SuggestedClassCache.Empty();
	IKCS_MonoAnalyzer::SuggestedFieldCache.Empty();
	IKCS_MonoAnalyzer::SuggestedMethodCache.Empty();
	IKCS_MonoAnalyzer::SuggestedPropertyCache.Empty();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SKCS_TextEditorWidget::SearchKeywordInfo() {
	if (HasSuggestion()&&(SuggestionResults.IsValidIndex(SuggestPicked))) {
		FCriticalSection Mutex;
		Mutex.Lock();
		//
		const FString &Keyword = SuggestionResults[SuggestPicked];
		FString Context = AutoKeyword;
		//
		if (IsAutoCompleteTree(Context)) {
			const FMonoNamespaceDefinition &SpaceInfo = IKCS_MonoAnalyzer::GetNamespaceInfo(ScriptSource.Get(),Keyword);
			//
			if (SpaceInfo.IsValid()) {
				FString Namespace{};
				//
				for (const auto &N : SpaceInfo.Namespaces) {
					Namespace.Append(N+TEXT("."));
				} Namespace.RemoveFromEnd(TEXT("."));
				//
				KeywordInfo = FString::Printf(TEXT("(%s)\n\n%s"),*SpaceInfo.Name.ToString(),*Namespace); return;
			}///
		} else {
			const FMonoNamespaceDefinition &SpaceInfo = IKCS_MonoAnalyzer::GetNamespaceInfo(ScriptSource.Get(),Keyword);
			//
			if (SpaceInfo.IsValid()) {
				FString Namespace{};
				//
				for (const auto &N : SpaceInfo.Namespaces) {
					Namespace.Append(N+TEXT("."));
				} Namespace.RemoveFromEnd(TEXT("."));
				//
				KeywordInfo = FString::Printf(TEXT("(%s)\n\n%s"),*SpaceInfo.Name.ToString(),*Namespace); return;
			}///
		}///
		//
		if (IsAutoCompleteTree(Context)) {
			Context.RemoveFromEnd(TEXT("."));
			{
				if (IKCS_MonoAnalyzer::SuggestedClassCache.Contains(Keyword)) {
					const FMonoClassDefinition &CacheInfo = IKCS_MonoAnalyzer::SuggestedClassCache.FindChecked(Keyword);
					if (CacheInfo.IsValid()) {KeywordInfo=FString::Printf(TEXT(" (%s)   %s\n\n%s"),*CacheInfo.ScopeToString(),*CacheInfo.Name.ToString(),*CacheInfo.Description); return;}
				}///
			}
			//
			TArray<FString>Tree{};
			for (const auto &Item : ParseAutoCompleteTree(Context)) {Tree.Add(FilterKeyword(Item));}
			const FMonoClassDefinition &ClassInfo = IKCS_MonoAnalyzer::GetClassInfo(ScriptSource.Get(),Tree,Keyword);
			if (ClassInfo.IsValid()) {KeywordInfo=FString::Printf(TEXT(" (%s)   %s\n\n%s"),*ClassInfo.ScopeToString(),*ClassInfo.Name.ToString(),*ClassInfo.Description); return;}
		} else {
			if (IKCS_MonoAnalyzer::SuggestedClassCache.Contains(Keyword)) {
				const FMonoClassDefinition &CacheInfo = IKCS_MonoAnalyzer::SuggestedClassCache.FindChecked(Keyword);
				if (CacheInfo.IsValid()) {KeywordInfo=FString::Printf(TEXT(" (%s)   %s\n\n%s"),*CacheInfo.ScopeToString(),*CacheInfo.Name.ToString(),*CacheInfo.Description); return;}
			}///
			//
			const FMonoClassDefinition &ClassInfo = IKCS_MonoAnalyzer::GetClassInfo(ScriptSource.Get(),FilterKeyword(Context),FilterKeyword(Keyword));
			if (ClassInfo.IsValid()) {KeywordInfo=FString::Printf(TEXT(" (%s)   %s\n\n%s"),*ClassInfo.ScopeToString(),*ClassInfo.Name.ToString(),*ClassInfo.Description); return;}
		}///
		//
		if (IsAutoCompleteTree(Context)) {
			Context.RemoveFromEnd(TEXT("."));
			{
				if (IKCS_MonoAnalyzer::SuggestedFieldCache.Contains(Keyword)) {
					const FMonoFieldDefinition &CacheInfo = IKCS_MonoAnalyzer::SuggestedFieldCache.FindChecked(Keyword);
					if (CacheInfo.IsValid()) {KeywordInfo=FString::Printf(TEXT(" %s(%s)   %s\n\n%s"),*CacheInfo.StackToString(),*CacheInfo.TypeToString(),*CacheInfo.Name.ToString(),*CacheInfo.Description); return;}
				}///
			}
			//
			TArray<FString>Tree{};
			for (const auto &Item : ParseAutoCompleteTree(Context)) {Tree.Add(FilterKeyword(Item));}
			const FMonoFieldDefinition &FieldInfo = IKCS_MonoAnalyzer::GetFieldInfo(ScriptSource.Get(),Tree,Keyword);
			if (FieldInfo.IsValid()) {KeywordInfo=FString::Printf(TEXT(" %s(%s)   %s\n\n%s"),*FieldInfo.StackToString(),*FieldInfo.TypeToString(),*FieldInfo.Name.ToString(),*FieldInfo.Description); return;}
		} else {
			if (IKCS_MonoAnalyzer::SuggestedFieldCache.Contains(Keyword)) {
				const FMonoFieldDefinition &CacheInfo = IKCS_MonoAnalyzer::SuggestedFieldCache.FindChecked(Keyword);
				if (CacheInfo.IsValid()) {KeywordInfo=FString::Printf(TEXT(" %s(%s)   %s\n\n%s"),*CacheInfo.StackToString(),*CacheInfo.TypeToString(),*CacheInfo.Name.ToString(),*CacheInfo.Description); return;}
			}///
			//
			const FMonoFieldDefinition &FieldInfo = IKCS_MonoAnalyzer::GetFieldInfo(ScriptSource.Get(),FilterKeyword(Context),FilterKeyword(Keyword));
			if (FieldInfo.IsValid()) {KeywordInfo=FString::Printf(TEXT(" %s(%s)   %s\n\n%s"),*FieldInfo.StackToString(),*FieldInfo.TypeToString(),*FieldInfo.Name.ToString(),*FieldInfo.Description); return;}
		}///
		//
		if (IsAutoCompleteTree(Context)) {
			Context.RemoveFromEnd(TEXT("."));
			{
				if (IKCS_MonoAnalyzer::SuggestedPropertyCache.Contains(Keyword)) {
					const FMonoFieldDefinition &CacheInfo = IKCS_MonoAnalyzer::SuggestedPropertyCache.FindChecked(Keyword);
					if (CacheInfo.IsValid()) {KeywordInfo=FString::Printf(TEXT(" %s(%s)   %s\n\n%s"),*CacheInfo.StackToString(),*CacheInfo.TypeToString(),*CacheInfo.Name.ToString(),*CacheInfo.Description); return;}
				}///
			}
			//
			TArray<FString>Tree{};
			for (const auto &Item : ParseAutoCompleteTree(Context)) {Tree.Add(FilterKeyword(Item));}
			const FMonoFieldDefinition &PropInfo = IKCS_MonoAnalyzer::GetPropertyInfo(ScriptSource.Get(),Tree,Keyword);
			if (PropInfo.IsValid()) {KeywordInfo=FString::Printf(TEXT(" %s(%s)   %s\n\n%s"),*PropInfo.StackToString(),*PropInfo.TypeToString(),*PropInfo.Name.ToString(),*PropInfo.Description); return;}
		} else {
			if (IKCS_MonoAnalyzer::SuggestedPropertyCache.Contains(Keyword)) {
				const FMonoFieldDefinition &CacheInfo = IKCS_MonoAnalyzer::SuggestedPropertyCache.FindChecked(Keyword);
				if (CacheInfo.IsValid()) {KeywordInfo=FString::Printf(TEXT(" %s(%s)   %s\n\n%s"),*CacheInfo.StackToString(),*CacheInfo.TypeToString(),*CacheInfo.Name.ToString(),*CacheInfo.Description); return;}
			}///
			//
			const FMonoFieldDefinition &PropInfo = IKCS_MonoAnalyzer::GetPropertyInfo(ScriptSource.Get(),FilterKeyword(Context),FilterKeyword(Keyword));
			if (PropInfo.IsValid()) {KeywordInfo=FString::Printf(TEXT(" %s(%s)   %s\n\n%s"),*PropInfo.StackToString(),*PropInfo.TypeToString(),*PropInfo.Name.ToString(),*PropInfo.Description); return;}
		}///
		//
		if (IsAutoCompleteTree(Context)) {
			Context.RemoveFromEnd(TEXT("."));
			{
				if (IKCS_MonoAnalyzer::SuggestedMethodCache.Contains(Keyword)) {
					const FMonoMethodDefinition &CacheInfo = IKCS_MonoAnalyzer::SuggestedMethodCache.FindChecked(Keyword);
					if (CacheInfo.IsValid()) {KeywordInfo=FString::Printf(TEXT(" (%s)   %s()\n\n%s"),*CacheInfo.TypeToString(),*CacheInfo.Name.ToString(),*CacheInfo.Description); return;}
				}///
			}
			//
			TArray<FString>Tree{};
			for (const auto &Item : ParseAutoCompleteTree(Context)) {Tree.Add(FilterKeyword(Item));}
			const FMonoMethodDefinition &MethodInfo = IKCS_MonoAnalyzer::GetMethodInfo(ScriptSource.Get(),Tree,Keyword);
			if (MethodInfo.IsValid()) {KeywordInfo=FString::Printf(TEXT(" (%s)   %s()\n\n%s"),*MethodInfo.TypeToString(),*MethodInfo.Name.ToString(),*MethodInfo.Description); return;}
		} else {
			if (IKCS_MonoAnalyzer::SuggestedMethodCache.Contains(Keyword)) {
				const FMonoMethodDefinition &CacheInfo = IKCS_MonoAnalyzer::SuggestedMethodCache.FindChecked(Keyword);
				if (CacheInfo.IsValid()) {KeywordInfo=FString::Printf(TEXT(" (%s)   %s()\n\n%s"),*CacheInfo.TypeToString(),*CacheInfo.Name.ToString(),*CacheInfo.Description); return;}
			}///
			//
			const FMonoMethodDefinition &MethodInfo = IKCS_MonoAnalyzer::GetMethodInfo(ScriptSource.Get(),FilterKeyword(Context),FilterKeyword(Keyword));
			if (MethodInfo.IsValid()) {KeywordInfo=FString::Printf(TEXT(" (%s)   %s()\n\n%s"),*MethodInfo.TypeToString(),*MethodInfo.Name.ToString(),*MethodInfo.Description); return;}
		}///
		//
		Mutex.Unlock();
	} else {KeywordInfo.Empty();}
}

const FSlateBrush* SKCS_TextEditorWidget::GetSuggestionIcon(const FString &Keyword) const {
	const FSlateBrush* Brush = FEditorStyle::GetBrush(TEXT("Kismet.VariableList.TypeIcon"));
	//
	if (HasSuggestion()) {
		FString Context = AutoKeyword;
		FCriticalSection Mutex;
		Mutex.Lock();
		//
		if (ScriptSource.IsValid()&&(ScriptSource->GetScriptName().Equals(Keyword,ESearchCase::CaseSensitive))) {
			Brush = FKCS_NodeStyle::Get().Get()->GetBrush(TEXT("KCS.Class.Public")); return Brush;
		}///
		//
		if (IsAutoCompleteTree(Context)) {
			const FMonoNamespaceDefinition &SpaceInfo = IKCS_MonoAnalyzer::GetNamespaceInfo(ScriptSource.Get(),Context);
			if (SpaceInfo.IsValid()) {Brush=FKCS_NodeStyle::Get().Get()->GetBrush(TEXT("KCS.Namespace")); return Brush;}
		} else {
			const FMonoNamespaceDefinition &SpaceInfo = IKCS_MonoAnalyzer::GetNamespaceInfo(ScriptSource.Get(),Context);
			if (SpaceInfo.IsValid()) {Brush=FKCS_NodeStyle::Get().Get()->GetBrush(TEXT("KCS.Namespace")); return Brush;}
		}///
		//
		if (IsAutoCompleteTree(Context)) {
			Context.RemoveFromEnd(TEXT("."));
			{
				if (IKCS_MonoAnalyzer::SuggestedClassCache.Contains(Keyword)) {
					const FMonoClassDefinition &CacheInfo = IKCS_MonoAnalyzer::SuggestedClassCache.FindChecked(Keyword);
					//
					if (CacheInfo.IsValid()) {
						switch (CacheInfo.AccessType) {
							case EMonoAccessType::Public: {Brush=FKCS_NodeStyle::Get().Get()->GetBrush(TEXT("KCS.Class.Public")); return Brush;}
							case EMonoAccessType::Private: {Brush=FKCS_NodeStyle::Get().Get()->GetBrush(TEXT("KCS.Class.Private")); return Brush;}
							case EMonoAccessType::Protected: {Brush=FKCS_NodeStyle::Get().Get()->GetBrush(TEXT("KCS.Class.Protected")); return Brush;}
						default: break;}
					}///
				}///
			}
			//
			TArray<FString>Tree{};
			for (const auto &Item : ParseAutoCompleteTree(Context)) {Tree.Add(FilterKeyword(Item));}
			//
			const FMonoNamespaceDefinition &SpaceInfo = IKCS_MonoAnalyzer::GetNamespaceInfo(ScriptSource.Get(),Tree,Keyword);
			if (SpaceInfo.IsValid()) {Brush=FKCS_NodeStyle::Get().Get()->GetBrush(TEXT("KCS.Namespace")); return Brush;}
			//
			const FMonoClassDefinition &ClassInfo = IKCS_MonoAnalyzer::GetClassInfo(ScriptSource.Get(),Tree,Keyword);
			//
			if (ClassInfo.IsValid()) {
				switch (ClassInfo.AccessType) {
					case EMonoAccessType::Public: {Brush=FKCS_NodeStyle::Get().Get()->GetBrush(TEXT("KCS.Class.Public")); return Brush;}
					case EMonoAccessType::Private: {Brush=FKCS_NodeStyle::Get().Get()->GetBrush(TEXT("KCS.Class.Private")); return Brush;}
					case EMonoAccessType::Protected: {Brush=FKCS_NodeStyle::Get().Get()->GetBrush(TEXT("KCS.Class.Protected")); return Brush;}
				default: break;}
			}///
		} else {
			if (IKCS_MonoAnalyzer::SuggestedClassCache.Contains(Keyword)) {
				const FMonoClassDefinition &CacheInfo = IKCS_MonoAnalyzer::SuggestedClassCache.FindChecked(Keyword);
				//
				if (CacheInfo.IsValid()) {
					switch (CacheInfo.AccessType) {
						case EMonoAccessType::Public: {Brush=FKCS_NodeStyle::Get().Get()->GetBrush(TEXT("KCS.Class.Public")); return Brush;}
						case EMonoAccessType::Private: {Brush=FKCS_NodeStyle::Get().Get()->GetBrush(TEXT("KCS.Class.Private")); return Brush;}
						case EMonoAccessType::Protected: {Brush=FKCS_NodeStyle::Get().Get()->GetBrush(TEXT("KCS.Class.Protected")); return Brush;}
					default: break;}
				}///
			}///
			//
			const FMonoNamespaceDefinition &SpaceInfo = IKCS_MonoAnalyzer::GetNamespaceInfo(ScriptSource.Get(),Keyword);
			if (SpaceInfo.IsValid()) {Brush=FKCS_NodeStyle::Get().Get()->GetBrush(TEXT("KCS.Namespace")); return Brush;}
			//
			const FMonoClassDefinition &ClassInfo = IKCS_MonoAnalyzer::GetClassInfo(ScriptSource.Get(),FilterKeyword(Context),FilterKeyword(Keyword));
			//
			if (ClassInfo.IsValid()) {
				switch (ClassInfo.AccessType) {
					case EMonoAccessType::Public: {Brush=FKCS_NodeStyle::Get().Get()->GetBrush(TEXT("KCS.Class.Public")); return Brush;}
					case EMonoAccessType::Private: {Brush=FKCS_NodeStyle::Get().Get()->GetBrush(TEXT("KCS.Class.Private")); return Brush;}
					case EMonoAccessType::Protected: {Brush=FKCS_NodeStyle::Get().Get()->GetBrush(TEXT("KCS.Class.Protected")); return Brush;}
				default: break;}
			}///
		}///
		//
		if (IsAutoCompleteTree(Context)) {
			Context.RemoveFromEnd(TEXT("."));
			{
				if (IKCS_MonoAnalyzer::SuggestedFieldCache.Contains(Keyword)) {
					const FMonoFieldDefinition &CacheInfo = IKCS_MonoAnalyzer::SuggestedFieldCache.FindChecked(Keyword);
					//
					if (CacheInfo.IsValid()) {
						switch (CacheInfo.AccessType) {
							case EMonoAccessType::Public: {Brush=FKCS_NodeStyle::Get().Get()->GetBrush(TEXT("KCS.Field.Public")); return Brush;}
							case EMonoAccessType::Private: {Brush=FKCS_NodeStyle::Get().Get()->GetBrush(TEXT("KCS.Field.Private")); return Brush;}
							case EMonoAccessType::Protected: {Brush=FKCS_NodeStyle::Get().Get()->GetBrush(TEXT("KCS.Field.Protected")); return Brush;}
						default: break;}
					}///
				}///
				//
				if (IKCS_MonoAnalyzer::SuggestedPropertyCache.Contains(Keyword)) {
					const FMonoFieldDefinition &CacheInfo = IKCS_MonoAnalyzer::SuggestedPropertyCache.FindChecked(Keyword);
					//
					if (CacheInfo.IsValid()) {
						switch (CacheInfo.AccessType) {
							case EMonoAccessType::Public: {Brush=FKCS_NodeStyle::Get().Get()->GetBrush(TEXT("KCS.Property.Public")); return Brush;}
							case EMonoAccessType::Private: {Brush=FKCS_NodeStyle::Get().Get()->GetBrush(TEXT("KCS.Property.Private")); return Brush;}
							case EMonoAccessType::Protected: {Brush=FKCS_NodeStyle::Get().Get()->GetBrush(TEXT("KCS.Property.Protected")); return Brush;}
						default: break;}
					}///
				}///
				//
				if (IKCS_MonoAnalyzer::SuggestedMethodCache.Contains(Keyword)) {
					const FMonoMethodDefinition &CacheInfo = IKCS_MonoAnalyzer::SuggestedMethodCache.FindChecked(Keyword);
					//
					if (CacheInfo.IsValid()) {
						switch (CacheInfo.AccessType) {
							case EMonoAccessType::Public: {Brush=FKCS_NodeStyle::Get().Get()->GetBrush(TEXT("KCS.Function.Public")); return Brush;}
							case EMonoAccessType::Private: {Brush=FKCS_NodeStyle::Get().Get()->GetBrush(TEXT("KCS.Function.Private")); return Brush;}
							case EMonoAccessType::Protected: {Brush=FKCS_NodeStyle::Get().Get()->GetBrush(TEXT("KCS.Function.Protected")); return Brush;}
						default: break;}
					}///
				}///
			}
			//
			TArray<FString>Tree{};
			for (const auto &Item : ParseAutoCompleteTree(Context)) {Tree.Add(FilterKeyword(Item));}
			const FMonoFieldDefinition &FieldInfo = IKCS_MonoAnalyzer::GetFieldInfo(ScriptSource.Get(),Tree,Keyword);
			//
			if (FieldInfo.IsValid()) {
				switch (FieldInfo.AccessType) {
					case EMonoAccessType::Public: {Brush=FKCS_NodeStyle::Get().Get()->GetBrush(TEXT("KCS.Field.Public")); return Brush;}
					case EMonoAccessType::Private: {Brush=FKCS_NodeStyle::Get().Get()->GetBrush(TEXT("KCS.Field.Private")); return Brush;}
					case EMonoAccessType::Protected: {Brush=FKCS_NodeStyle::Get().Get()->GetBrush(TEXT("KCS.Field.Protected")); return Brush;}
				default: break;}
			}///
			//
			const FMonoFieldDefinition &PropInfo = IKCS_MonoAnalyzer::GetPropertyInfo(ScriptSource.Get(),Tree,Keyword);
			//
			if (PropInfo.IsValid()) {
				switch (PropInfo.AccessType) {
					case EMonoAccessType::Public: {Brush=FKCS_NodeStyle::Get().Get()->GetBrush(TEXT("KCS.Property.Public")); return Brush;}
					case EMonoAccessType::Private: {Brush=FKCS_NodeStyle::Get().Get()->GetBrush(TEXT("KCS.Property.Private")); return Brush;}
					case EMonoAccessType::Protected: {Brush=FKCS_NodeStyle::Get().Get()->GetBrush(TEXT("KCS.Property.Protected")); return Brush;}
				default: break;}
			}///
			//
			const FMonoMethodDefinition &MethodInfo = IKCS_MonoAnalyzer::GetMethodInfo(ScriptSource.Get(),Tree,Keyword);
			//
			if (MethodInfo.IsValid()) {
				switch (MethodInfo.AccessType) {
					case EMonoAccessType::Public: {Brush=FKCS_NodeStyle::Get().Get()->GetBrush(TEXT("KCS.Function.Public")); return Brush;}
					case EMonoAccessType::Private: {Brush=FKCS_NodeStyle::Get().Get()->GetBrush(TEXT("KCS.Function.Private")); return Brush;}
					case EMonoAccessType::Protected: {Brush=FKCS_NodeStyle::Get().Get()->GetBrush(TEXT("KCS.Function.Protected")); return Brush;}
				default: break;}
			}///
		} else {
			if (IKCS_MonoAnalyzer::SuggestedFieldCache.Contains(Keyword)) {
				const FMonoFieldDefinition &CacheInfo = IKCS_MonoAnalyzer::SuggestedFieldCache.FindChecked(Keyword);
				//
				if (CacheInfo.IsValid()) {
					switch (CacheInfo.AccessType) {
						case EMonoAccessType::Public: {Brush=FKCS_NodeStyle::Get().Get()->GetBrush(TEXT("KCS.Field.Public")); return Brush;}
						case EMonoAccessType::Private: {Brush=FKCS_NodeStyle::Get().Get()->GetBrush(TEXT("KCS.Field.Private")); return Brush;}
						case EMonoAccessType::Protected: {Brush=FKCS_NodeStyle::Get().Get()->GetBrush(TEXT("KCS.Field.Protected")); return Brush;}
					default: break;}
				}///
			}///
			//
			if (IKCS_MonoAnalyzer::SuggestedPropertyCache.Contains(Keyword)) {
				const FMonoFieldDefinition &CacheInfo = IKCS_MonoAnalyzer::SuggestedPropertyCache.FindChecked(Keyword);
				//
				if (CacheInfo.IsValid()) {
					switch (CacheInfo.AccessType) {
						case EMonoAccessType::Public: {Brush=FKCS_NodeStyle::Get().Get()->GetBrush(TEXT("KCS.Property.Public")); return Brush;}
						case EMonoAccessType::Private: {Brush=FKCS_NodeStyle::Get().Get()->GetBrush(TEXT("KCS.Property.Private")); return Brush;}
						case EMonoAccessType::Protected: {Brush=FKCS_NodeStyle::Get().Get()->GetBrush(TEXT("KCS.Property.Protected")); return Brush;}
					default: break;}
				}///
			}///
			//
			if (IKCS_MonoAnalyzer::SuggestedMethodCache.Contains(Keyword)) {
				const FMonoMethodDefinition &CacheInfo = IKCS_MonoAnalyzer::SuggestedMethodCache.FindChecked(Keyword);
				//
				if (CacheInfo.IsValid()) {
					switch (CacheInfo.AccessType) {
						case EMonoAccessType::Public: {Brush=FKCS_NodeStyle::Get().Get()->GetBrush(TEXT("KCS.Function.Public")); return Brush;}
						case EMonoAccessType::Private: {Brush=FKCS_NodeStyle::Get().Get()->GetBrush(TEXT("KCS.Function.Private")); return Brush;}
						case EMonoAccessType::Protected: {Brush=FKCS_NodeStyle::Get().Get()->GetBrush(TEXT("KCS.Function.Protected")); return Brush;}
					default: break;}
				}///
			}///
			//
			const FMonoFieldDefinition &FieldInfo = IKCS_MonoAnalyzer::GetFieldInfo(ScriptSource.Get(),FilterKeyword(Context),FilterKeyword(Keyword));
			//
			if (FieldInfo.IsValid()) {
				switch (FieldInfo.AccessType) {
					case EMonoAccessType::Public: {Brush=FKCS_NodeStyle::Get().Get()->GetBrush(TEXT("KCS.Field.Public")); return Brush;}
					case EMonoAccessType::Private: {Brush=FKCS_NodeStyle::Get().Get()->GetBrush(TEXT("KCS.Field.Private")); return Brush;}
					case EMonoAccessType::Protected: {Brush=FKCS_NodeStyle::Get().Get()->GetBrush(TEXT("KCS.Field.Protected")); return Brush;}
				default: break;}
			}///
			//
			const FMonoFieldDefinition &PropInfo = IKCS_MonoAnalyzer::GetPropertyInfo(ScriptSource.Get(),FilterKeyword(Context),FilterKeyword(Keyword));
			//
			if (PropInfo.IsValid()) {
				switch (PropInfo.AccessType) {
					case EMonoAccessType::Public: {Brush=FKCS_NodeStyle::Get().Get()->GetBrush(TEXT("KCS.Property.Public")); return Brush;}
					case EMonoAccessType::Private: {Brush=FKCS_NodeStyle::Get().Get()->GetBrush(TEXT("KCS.Property.Private")); return Brush;}
					case EMonoAccessType::Protected: {Brush=FKCS_NodeStyle::Get().Get()->GetBrush(TEXT("KCS.Property.Protected")); return Brush;}
				default: break;}
			}///
			//
			const FMonoMethodDefinition &MethodInfo = IKCS_MonoAnalyzer::GetMethodInfo(ScriptSource.Get(),FilterKeyword(Context),FilterKeyword(Keyword));
			//
			if (MethodInfo.IsValid()) {
				switch (MethodInfo.AccessType) {
					case EMonoAccessType::Public: {Brush=FKCS_NodeStyle::Get().Get()->GetBrush(TEXT("KCS.Function.Public")); return Brush;}
					case EMonoAccessType::Private: {Brush=FKCS_NodeStyle::Get().Get()->GetBrush(TEXT("KCS.Function.Private")); return Brush;}
					case EMonoAccessType::Protected: {Brush=FKCS_NodeStyle::Get().Get()->GetBrush(TEXT("KCS.Function.Protected")); return Brush;}
				default: break;}
			}///
		}///
		//
		Mutex.Unlock();
	}///
	//
	return Brush;
}

const FLinearColor SKCS_TextEditorWidget::GetSuggestionColor(const FString &Keyword) const {
	FLinearColor Color = FLinearColor::White;
	//
	if (HasSuggestion()) {
		FCriticalSection Mutex;
		Mutex.Lock();
		//
		if (ScriptSource.IsValid()&&(ScriptSource->GetScriptName().Equals(Keyword,ESearchCase::CaseSensitive))) {
			Color = FLinearColor(FColor(255,235,205)); return Color;
		}///
		//
		FString Context = AutoKeyword;
		if (IsAutoCompleteTree(Context)) {
			Context.RemoveFromEnd(TEXT("."));
			{
				if (IKCS_MonoAnalyzer::SuggestedClassCache.Contains(Keyword)) {
					const FMonoClassDefinition &CacheInfo = IKCS_MonoAnalyzer::SuggestedClassCache.FindChecked(Keyword);
					if (CacheInfo.IsValid()) {Color=FLinearColor(FColor(255,235,205)); return Color;}
				}///
			}
			//
			TArray<FString>Tree{};
			for (const auto &Item : ParseAutoCompleteTree(Context)) {Tree.Add(FilterKeyword(Item));}
			const FMonoNamespaceDefinition &SpaceInfo = IKCS_MonoAnalyzer::GetNamespaceInfo(ScriptSource.Get(),Tree,Keyword);
			if (SpaceInfo.IsValid()) {Color=FLinearColor(FColor(225,225,255)); return Color;}
			//
			const FMonoClassDefinition &ClassInfo = IKCS_MonoAnalyzer::GetClassInfo(ScriptSource.Get(),Tree,Keyword);
			if (ClassInfo.IsValid()) {Color=FLinearColor(FColor(255,235,205)); return Color;}
		} else {
			if (IKCS_MonoAnalyzer::SuggestedClassCache.Contains(Keyword)) {
				const FMonoClassDefinition &CacheInfo = IKCS_MonoAnalyzer::SuggestedClassCache.FindChecked(Keyword);
				if (CacheInfo.IsValid()) {Color=FLinearColor(FColor(255,235,205)); return Color;}
			}///
			//
			const FMonoNamespaceDefinition &SpaceInfo = IKCS_MonoAnalyzer::GetNamespaceInfo(ScriptSource.Get(),Keyword);
			if (SpaceInfo.IsValid()) {Color=FLinearColor(FColor(225,225,255)); return Color;}
			//
			const FMonoClassDefinition &ClassInfo = IKCS_MonoAnalyzer::GetClassInfo(ScriptSource.Get(),FilterKeyword(Context),FilterKeyword(Keyword));
			if (ClassInfo.IsValid()) {Color=FLinearColor(FColor(255,235,205)); return Color;}
		}///
		//
		if (IsAutoCompleteTree(Context)) {
			Context.RemoveFromEnd(TEXT("."));
			{
				if (IKCS_MonoAnalyzer::SuggestedFieldCache.Contains(Keyword)) {
					const FMonoFieldDefinition &CacheInfo = IKCS_MonoAnalyzer::SuggestedFieldCache.FindChecked(Keyword);
					if (CacheInfo.IsValid()) {Color=FLinearColor(FColor(255,235,205)); return Color;}
				}///
			}
			//
			TArray<FString>Tree{};
			for (const auto &Item : ParseAutoCompleteTree(Context)) {Tree.Add(FilterKeyword(Item));}
			const FMonoFieldDefinition &FieldInfo = IKCS_MonoAnalyzer::GetFieldInfo(ScriptSource.Get(),Tree,Keyword);
			if (FieldInfo.IsValid()) {Color=FLinearColor(FColor(205,255,255)); return Color;}
		} else {
			if (IKCS_MonoAnalyzer::SuggestedFieldCache.Contains(Keyword)) {
				const FMonoFieldDefinition &CacheInfo = IKCS_MonoAnalyzer::SuggestedFieldCache.FindChecked(Keyword);
				if (CacheInfo.IsValid()) {Color=FLinearColor(FColor(255,235,205)); return Color;}
			}///
			//
			const FMonoFieldDefinition &FieldInfo = IKCS_MonoAnalyzer::GetFieldInfo(ScriptSource.Get(),FilterKeyword(Context),FilterKeyword(Keyword));
			if (FieldInfo.IsValid()) {Color=FLinearColor(FColor(205,255,255)); return Color;}
		}///
		//
		if (IsAutoCompleteTree(Context)) {
			Context.RemoveFromEnd(TEXT("."));
			{
				if (IKCS_MonoAnalyzer::SuggestedPropertyCache.Contains(Keyword)) {
					const FMonoFieldDefinition &CacheInfo = IKCS_MonoAnalyzer::SuggestedPropertyCache.FindChecked(Keyword);
					if (CacheInfo.IsValid()) {Color=FLinearColor(FColor(255,235,205)); return Color;}
				}///
			}
			//
			TArray<FString>Tree{};
			for (const auto &Item : ParseAutoCompleteTree(Context)) {Tree.Add(FilterKeyword(Item));}
			const FMonoFieldDefinition &PropInfo = IKCS_MonoAnalyzer::GetPropertyInfo(ScriptSource.Get(),Tree,Keyword);
			if (PropInfo.IsValid()) {Color=FLinearColor(FColor(205,255,225)); return Color;}
		} else {
			if (IKCS_MonoAnalyzer::SuggestedPropertyCache.Contains(Keyword)) {
				const FMonoFieldDefinition &CacheInfo = IKCS_MonoAnalyzer::SuggestedPropertyCache.FindChecked(Keyword);
				if (CacheInfo.IsValid()) {Color=FLinearColor(FColor(255,235,205)); return Color;}
			}///
			//
			const FMonoFieldDefinition &PropInfo = IKCS_MonoAnalyzer::GetPropertyInfo(ScriptSource.Get(),FilterKeyword(Context),FilterKeyword(Keyword));
			if (PropInfo.IsValid()) {Color=FLinearColor(FColor(205,255,225)); return Color;}
		}///
		//
		if (IsAutoCompleteTree(Context)) {
			Context.RemoveFromEnd(TEXT("."));
			{
				if (IKCS_MonoAnalyzer::SuggestedMethodCache.Contains(Keyword)) {
					const FMonoMethodDefinition &CacheInfo = IKCS_MonoAnalyzer::SuggestedMethodCache.FindChecked(Keyword);
					if (CacheInfo.IsValid()) {Color=FLinearColor(FColor(255,235,205)); return Color;}
				}///
			}
			//
			TArray<FString>Tree{};
			for (const auto &Item : ParseAutoCompleteTree(Context)) {Tree.Add(FilterKeyword(Item));}
			const FMonoMethodDefinition &MethodInfo = IKCS_MonoAnalyzer::GetMethodInfo(ScriptSource.Get(),Tree,Keyword);
			if (MethodInfo.IsValid()) {Color=FLinearColor(FColor(255,225,205)); return Color;}
		} else {
			if (IKCS_MonoAnalyzer::SuggestedMethodCache.Contains(Keyword)) {
				const FMonoMethodDefinition &CacheInfo = IKCS_MonoAnalyzer::SuggestedMethodCache.FindChecked(Keyword);
				if (CacheInfo.IsValid()) {Color=FLinearColor(FColor(255,235,205)); return Color;}
			}///
			//
			const FMonoMethodDefinition &MethodInfo = IKCS_MonoAnalyzer::GetMethodInfo(ScriptSource.Get(),FilterKeyword(Context),FilterKeyword(Keyword));
			if (MethodInfo.IsValid()) {Color=FLinearColor(FColor(255,225,205)); return Color;}
		}///
		//
		Mutex.Unlock();
	}///
	//
	return Color;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SKCS_TextEditorWidget::FormatSourceCode() {
	static IMagicNodeSharpKismet &MonoKismet = FMagicNodeSharpKismet::Get();
	static IMagicNodeSharp &MonoCore = FMagicNodeSharp::Get();
	//
	if (MonoKismet.CanFormatCode()) {
		void* Args[1];
		{
			MonoString* CodeInterOPS = mono_string_new(MonoCore.GetCoreDomain(),StringCast<ANSICHAR>(*ScriptSource->GetSource()).Get());
			Args[0] = CodeInterOPS;
		}
		//
		MonoString* RCall = (MonoString*)mono_runtime_invoke(MonoKismet.GetCodeFormatMethod(),NULL,Args,NULL);
		//
		WIDECHAR* SCR = mono_string_to_utf16(RCall);
		FString STR = StringCast<TCHAR>(SCR).Get();
		//
		if (!STR.IsEmpty()) {
			BeginEditTransaction();
			SetText(FText::FromString(STR));
			EndEditTransaction();
		}///
		//
		mono_free(SCR);
	}///
	//
	CountLines();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SKCS_TextEditorWidget::SetScriptSource(UMagicNodeSharpSource* Script) {
	ScriptSource = Script;
}

void SKCS_TextEditorWidget::BeginEditTransaction() {
	if (!EditableTextLayout.IsValid()) {return;}
	//
	EditableTextLayout->BeginEditTransation();
}

void SKCS_TextEditorWidget::EndEditTransaction() {
	if (!EditableTextLayout.IsValid()) {return;}
	//
	EditableTextLayout->EndEditTransaction();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////