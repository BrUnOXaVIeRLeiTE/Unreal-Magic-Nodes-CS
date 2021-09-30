//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
///			Copyright 2021 (C) Bruno Xavier B. Leite
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "MagicNodeSharp.h"
#include "KCS_TextEditorWidget.h"

#include "Runtime/SlateCore/Public/Widgets/SOverlay.h"
#include "Runtime/SlateCore/Public/Widgets/Images/SImage.h"

#include "Runtime/Slate/Public/Widgets/Layout/SBox.h"
#include "Runtime/Slate/Public/Widgets/Input/SButton.h"
#include "Runtime/Slate/Public/Widgets/Layout/SBorder.h"
#include "Runtime/Slate/Public/Widgets/Views/SListView.h"
#include "Runtime/Slate/Public/Widgets/Input/SCheckBox.h"
#include "Runtime/Slate/Public/Widgets/Layout/SScrollBar.h"
#include "Runtime/Slate/Public/Widgets/Layout/SScrollBox.h"
#include "Runtime/Slate/Public/Widgets/Input/SEditableText.h"
#include "Runtime/Slate/Public/Widgets/Input/SEditableTextBox.h"
#include "Runtime/Slate/Public/Widgets/Notifications/SProgressBar.h"

#include "Editor/UnrealEd/Public/FileHelpers.h"
#include "Editor/GraphEditor/Public/SGraphNode.h"
#include "Editor/EditorStyle/Public/EditorStyle.h"
#include "Editor/UnrealEd/Public/WorkflowOrientedApp/SContentReference.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class FTextSyntaxHighlighter;
class UMagicNodeSharpSource;
class UKCS_MagicNode;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class SKCS_MagicNodeWidget : public SGraphNode {
private:
	TSharedPtr<FTextSyntaxHighlighter>MARSHALL;
	//
	TSharedPtr<SListView<TSharedPtr<FString>>>LINE_COUNTER;
	TSharedPtr<SKCS_TextEditorWidget>SCRIPT_EDITOR;
	TSharedPtr<SContentReference>SCRIPT_PICKER;
	TSharedPtr<SEditableTextBox>SEARCH_TEXT;
	TSharedPtr<SScrollBox>VS_SCROLL_BOX;
	TSharedPtr<SScrollBar>HSS_SCROLL;
	TSharedPtr<SScrollBar>VS_SCROLL;
	//
	TArray<TSharedPtr<FString>>LineCount;
	//
	FString ReplaceText;
	FString SearchText;
	FString LastHint;
	//
	bool ViewMacros;
	bool ViewIncludes;
	bool ViewSearchBox;
	bool SensitiveSearch;
	//
	int32 ErrorLine;
	float HintTimer;
	float CompilerProgress;
public:
	SKCS_MagicNodeWidget();
	virtual ~SKCS_MagicNodeWidget() override;
	//
	SLATE_BEGIN_ARGS(SKCS_MagicNodeWidget){}
	SLATE_END_ARGS()
	//
	void Construct(const FArguments &InArgs, UKCS_MagicNode* InGraphNode);
	//
	//
	bool HasScript() const;
	bool IsCompiling() const;
	bool CanCompileScript() const;
	bool IsScriptSourceLocked() const;
	bool IsScriptSourceEditable() const;
	//
	int32 GetLineCount() const;
	FText GetScriptText() const;
	FText GetSearchText() const;
	FText GetReplaceText() const;
	FText GetCursorLocation() const;
	//
	ECheckBoxState IsSearchSensitive() const;
	//
	//
	UObject* GetScriptSource() const;
	TOptional<float>GetCompilerProgress() const;
	//
	//
	FReply OnClickedCompile();
	FReply OnClickedSaveScript();
	FReply OnClickedSearchGlass();
	FReply OnClickedReloadScript();
	FReply OnClickedReplaceSearch();
	//
	EVisibility GetSearchBoxVisibility() const;
	EVisibility GetScriptEditorVisibility() const;
	EVisibility GetCompilerProgressVisibility() const;
	//
	const FSlateBrush* GetCompilerIcon() const;
	//
	TSharedRef<ITableRow>OnGenerateLineCounter(TSharedPtr<FString>Item, const TSharedRef<STableViewBase>&OwnerTable);
	//
	void OnVerticalScroll(float Offset);
	void OnScriptHorizontalScroll(float Offset);
	void OnInternalVerticalScroll(float Offset);
	//
	void OnInvokedSearch(bool DoSearch);
	void OnSearchSensitiveChanged(ECheckBoxState NewState);
	//
	void OnScriptTextChanged(const FText &InText, ETextCommit::Type CommitType);
	void OnSearchTextChanged(const FText &InText, ETextCommit::Type CommitType);
	void OnScriptTextComitted(const FText &NewText, ETextCommit::Type CommitInfo);
	void OnSearchTextComitted(const FText &NewText, ETextCommit::Type CommitInfo);
	void OnReplaceTextComitted(const FText &NewText, ETextCommit::Type CommitInfo);
	//
	//
	void OnSelectedLineCounterItem(TSharedPtr<FString>Item, ESelectInfo::Type SelectInfo);
	//
	void SetScriptSource(UObject* New);
	void SetScriptText(const FText &NewText);
	void SetLineCountList(const int32 Count);
	//
	void SetErrorMessage(const FString &NewError);
	void SetErrorMessage(const FCompilerResults &NewResult);
	//
	//
	virtual void UpdateGraphNode() override;
	virtual void CreateBelowWidgetControls(TSharedPtr<SVerticalBox>MainBox) override;
	virtual void Tick(const FGeometry &Geometry, const double CurrentTime, const float DeltaTime) override;
	//
	virtual bool IsInteractable() const override;
	virtual bool SupportsKeyboardFocus() const override;
	//
	virtual FReply OnKeyDown(const FGeometry &Geometry, const FKeyEvent &KeyEvent) override;
	virtual FReply OnMouseMove(const FGeometry &Geometry, const FPointerEvent &MouseEvent) override;
	//
	//
	void UpdateTextEditorScriptReference();
	void UpdateTextEditorScriptReference(UMagicNodeSharpSource* Script);
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////