//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
///			Copyright 2021 (C) Bruno Xavier B. Leite
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "MagicNodeSharpKismet_Shared.h"
#include "MCS_Types.h"

#include "Input/Reply.h"
#include "Fonts/FontMeasure.h"

#include "Styling/CoreStyle.h"
#include "Styling/SlateTypes.h"

#include "Runtime/SlateCore/Public/Widgets/Images/SImage.h"
#include "Runtime/Slate/Public/Widgets/Layout/SScrollBar.h"
#include "Runtime/Slate/Public/Widgets/Text/SMultiLineEditableText.h"
#include "Runtime/Slate/Public/Widgets/Text/SlateEditableTextLayout.h"
#include "Runtime/SlateCore/Public/Widgets/DeclarativeSyntaxSupport.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

DECLARE_DELEGATE_OneParam(FOnInvokeSearchEvent,bool);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class FTextSyntaxHighlighter;
class UMagicNodeSharpSource;
class UMagicNodeScript;
class UKCS_MagicNode;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class MAGICNODESHARPKISMET_API SKCS_TextEditorWidget : public SMultiLineEditableText {
private:
	TWeakObjectPtr<UMagicNodeSharpSource>ScriptSource;
	//
	EMonoCompilerResult ErrorType;
	//
	int32 ErrorLine;
	int32 LineCount;
	int32 NextLineFeed;
	int32 LastLineFeed;
	int32 SuggestPicked;
	int32 SuggestRootID;
	int32 SuggestFocusID;
	//
	float TypeWidth;
	float LineHeight;
	//
	bool ShowSearchBox;
	bool KeyboardFocus;
private:
	const bool IsTab(const TCHAR &CH) const;
	const bool IsBracket(const TCHAR &CH) const;
	const bool IsAngular(const TCHAR &CH) const;
	const bool IsOperator(const TCHAR &CH) const;
	const bool IsParentheses(const TCHAR &CH) const;
	const bool IsOpenBracket(const TCHAR &CH) const;
	const bool IsCloseBracket(const TCHAR &CH) const;
protected:
	TSharedPtr<FTextSyntaxHighlighter>Marshall;
	TSharedPtr<FSlateFontMeasure>FontMeasure;
	TSharedPtr<SScrollBar>VScroll;
	//
	FOnCursorMoved OnMovedCursor;
	FOnInvokeSearchEvent OnInvokedSearch;
	//
	FVector2D CompletionBoxSize;
	FVector2D CompletionBoxPos;
	FVector2D MousePosition;
	//
	FGeometry LastTickGeometry;
	//
	FTextLocation LastPosition;
	FTextLocation CursorLocation;
	//
	TArray<FString>SuggestionResults;
	//
	mutable FString CurrentLine;
	mutable FString UnderCursor;
	mutable FString AutoKeyword;
	mutable FString KeywordInfo;
public:
	SLATE_BEGIN_ARGS(SKCS_TextEditorWidget)
	{}///
		SLATE_ARGUMENT(TSharedPtr<SMultiLineEditableText>,LineCounter)
		SLATE_ARGUMENT(TSharedPtr<FTextSyntaxHighlighter>,Marshaller)
		SLATE_ARGUMENT(TSharedPtr<SScrollBar>,HScrollBar)
		SLATE_ARGUMENT(TSharedPtr<SScrollBar>,VScrollBar)
		SLATE_ATTRIBUTE(bool,CanKeyboardFocus)
		SLATE_ATTRIBUTE(bool,IsReadOnly)
		SLATE_ATTRIBUTE(FText,Text)
		//
		SLATE_EVENT(FOnInvokeSearchEvent,OnInvokeSearch)
		SLATE_EVENT(FOnTextCommitted,OnTextCommitted)
		SLATE_EVENT(FOnTextChanged,OnTextChanged)
		SLATE_EVENT(FOnCursorMoved,OnCursorMoved)
	SLATE_END_ARGS()
	//
	//
	void Construct(const FArguments &InArgs);
	virtual void Tick(const FGeometry &AllottedGeometry, const double CurrentTime, const float DeltaTime) override;
	virtual int32 OnPaint(const FPaintArgs &Args, const FGeometry &Geometry, const FSlateRect &CullingRect, FSlateWindowElementList &OutDrawElements, int32 LayerID, const FWidgetStyle &WidgetStyle, bool ParentEnabled) const override;
	//
	virtual bool SupportsKeyboardFocus() const override {return KeyboardFocus;}
	//
	virtual FReply OnKeyUp(const FGeometry &Geometry, const FKeyEvent &KeyEvent) override;
	virtual FReply OnKeyDown(const FGeometry &Geometry, const FKeyEvent &KeyEvent) override;
	virtual FReply OnMouseMove(const FGeometry &Geometry, const FPointerEvent &MouseEvent) override;
	virtual FReply OnMouseWheel(const FGeometry &Geometry, const FPointerEvent &MouseEvent) override;
	virtual FReply OnKeyChar(const FGeometry &Geometry, const FCharacterEvent &CharacterEvent) override;
	virtual FReply OnMouseButtonUp(const FGeometry &Geometry, const FPointerEvent &MouseEvent) override;
	virtual FReply OnMouseButtonDown(const FGeometry &Geometry, const FPointerEvent &MouseEvent) override;
	//
	void OnTextCursorMoved(const FTextLocation &NewPosition);
public:
	void SelectLine();
	void DeleteSelectedText();
	void GoToLineColumn(int32 Line, int32 Column);
public:
	const int32 GetLineCount() const;
	const FString &CaptureCursor() const;
	const FString &GetUnderCursor() const;
	const FTextLocation &GetCursorLocation() const;
public:
	const FVector2D GetScrollOffset() const;
	const FLinearColor GetLineIndexColor(int32 Line) const;
public:
	int32 CountLines() const;
	int32 CountSelectedLines() const;
	int32 CountTabs(bool BreakOnAlpha=false) const;
	int32 CountChars(const TCHAR &CH, const FString &Word) const;
public:
	FVector2D CalculateWordSize() const;
	FVector2D GetCompletionBoxPos() const;
	FVector2D GetCompletionBoxSize() const;
public:
	const FString ParseAutoCompleteWord() const;
	const FString FilterKeyword(const FString &Keyword) const;
	const TArray<FString>ParseAutoCompleteTree(const FString &Keyword) const;
public:
	const FString &GetKeywordInfo() const;
	const FString &GetCurrentLineAtCursor() const;
	const FString &GetAutoCompleteSubject() const;
public:
	const FSlateBrush* GetSuggestionIcon(const FString &Keyword) const;
	const FLinearColor GetSuggestionColor(const FString &Keyword) const;
public:
	const bool HasSuggestion() const;
	const bool IsAutoComplete(const FString &Keyword) const;
	const bool IsAutoCompleteTree(const FString &Keyword) const;
public:
	void AutoSuggest();
	void SuggestKeyword();
	void ClearSuggestion();
	void FilterSuggestion();
	void FormatSourceCode();
	void SearchKeywordInfo();
	void AutoSuggestCompleted();
	void InsertPickedSuggestion();
public:
	void DoAutoSuggestion(const FString &Keyword);
public:
	void SetScriptSource(UMagicNodeSharpSource* Script);
	void SetErrorLine(int32 NewLine) {ErrorLine=NewLine;}
	void SetErrorType(const EMonoCompilerResult &Result) {ErrorType=Result;}
	//
	void BeginEditTransaction();
	void EndEditTransaction();
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////