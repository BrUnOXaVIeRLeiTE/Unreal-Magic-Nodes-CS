//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
///			Copyright 2021 (C) Bruno Xavier B. Leite
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "KCS_TextEditorWidget.h"

#include "Runtime/Slate/Public/Widgets/Layout/SBox.h"
#include "Runtime/SlateCore/Public/Widgets/SOverlay.h"
#include "Runtime/Slate/Public/Widgets/Input/SButton.h"
#include "Runtime/Slate/Public/Widgets/Layout/SBorder.h"
#include "Runtime/Slate/Public/Widgets/Views/SListView.h"
#include "Runtime/Slate/Public/Widgets/Input/SCheckBox.h"
#include "Runtime/Slate/Public/Widgets/Layout/SScrollBar.h"
#include "Runtime/Slate/Public/Widgets/Layout/SScrollBox.h"
#include "Runtime/SlateCore/Public/Widgets/Images/SImage.h"
#include "Runtime/Slate/Public/Widgets/Input/SEditableText.h"
#include "Runtime/Slate/Public/Widgets/Input/SEditableTextBox.h"

#include "Editor/EditorStyle/Public/EditorStyle.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class UMagicNodeSharpSource;
class FKCS_NodeStyle;
class UMagicNode;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class SCS_CodeEditorCore : public SCompoundWidget {
private:
	TSharedPtr<SEditableTextBox>SEARCH_TEXT;
	TSharedPtr<FTextSyntaxHighlighter>MARSHALL;
	TSharedPtr<SKCS_TextEditorWidget>SCRIPT_EDITOR;
	TSharedPtr<SListView<TSharedPtr<FString>>>LINE_COUNTER;
	//
	TSharedPtr<SScrollBox>VS_SCROLL_BOX;
	TSharedPtr<SScrollBar>HS_SCROLL;
	TSharedPtr<SScrollBar>VS_SCROLL;
	//
	UMagicNodeSharpSource* ScriptSource;
private:
	TArray<TSharedPtr<FString>>LineCount;
	//
	FString SearchText;
	FString ReplaceText;
	FString KeywordInfo;
	//
	int32 ErrorLine;
	FTextLocation ZeroLocation;
	EMonoCompilerResult ErrorType;
	//
	float HintTimer;
	bool ViewSearchBox;
	bool SensitiveSearch;
public:
	SCS_CodeEditorCore();
	virtual ~SCS_CodeEditorCore();
	//
	SLATE_BEGIN_ARGS(SCS_CodeEditorCore)
	{}
	SLATE_END_ARGS()
public:
	void Construct(const FArguments &InArgs, UMagicNodeSharpSource* InScriptSource);
public:
	virtual void Tick(const FGeometry &AllottedGeometry, const double CurrentTime, const float DeltaTime) override;
	virtual FReply OnMouseMove(const FGeometry &Geometry, const FPointerEvent &MouseEvent) override;
	virtual FReply OnKeyDown(const FGeometry &Geometry, const FKeyEvent &KeyEvent) override;
public:
	bool HasScript() const;
	bool IsSourceLocked() const;
	bool IsScriptEditable() const;
public:
	int32 GetLineCount() const;
	FText GetScriptText() const;
	FText GetSearchText() const;
	FText GetReplaceText() const;
	FText GetCursorLocation() const;
	FText GetHintKeywordInfo() const;
public:
	const FTextLocation &GetCursorOffset() const;
public:
	ECheckBoxState IsSearchSensitive() const;
	EVisibility GetHintBoxVisibility() const;
	EVisibility GetSearchBoxVisibility() const;
public:
	void SetScriptText(const FText &NewText);
	void SetLineCountList(const int32 Count);
	void SetScriptError(const EMonoCompilerResult &NewResult, const FSourceInfo &Info);
public:
	void GoToTextLocation(const FTextLocation &Location);
public:
	void OnScriptTextChanged(const FText &InText, ETextCommit::Type CommitType);
	void OnScriptTextComitted(const FText &NewText, ETextCommit::Type CommitInfo);
	void OnReplaceTextComitted(const FText &NewText, ETextCommit::Type CommitInfo);
	void OnSelectedLineCounterItem(TSharedPtr<FString>Item, ESelectInfo::Type SelectInfo);
	//
	void OnInvokedSearch(bool DoSearch);
	void OnSearchSensitiveChanged(ECheckBoxState NewState);
	void OnSearchTextChanged(const FText &InText, ETextCommit::Type CommitType);
	void OnSearchTextComitted(const FText &NewText, ETextCommit::Type CommitInfo);
	//
	void OnVerticalScroll(float Offset);
	void OnInternalVerticalScroll(float Offset);
	void OnScriptHorizontalScroll(float Offset);
public:
	FReply OnClickedSearchGlass();
	FReply OnClickedReplaceSearch();
public:
	TSharedRef<ITableRow>OnGenerateLineCounter(TSharedPtr<FString>Item, const TSharedRef<STableViewBase>&OwnerTable);
public:
	void UpdateTextEditorScriptReference();
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////