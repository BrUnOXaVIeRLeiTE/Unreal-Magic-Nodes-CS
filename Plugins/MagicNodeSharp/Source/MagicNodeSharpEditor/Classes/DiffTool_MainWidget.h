/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
///			Copyright 2021 (C) Bruno Xavier B. Leite
//////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "MagicNodeSharpEditor.h"
#include "DiffTool_ListItem.h"

#include "Runtime/Slate/Public/Widgets/Layout/SBox.h"
#include "Runtime/Slate/Public/Widgets/Input/SButton.h"
#include "Runtime/Slate/Public/Widgets/Layout/SBorder.h"
#include "Runtime/Slate/Public/Widgets/Views/SListView.h"
#include "Runtime/Slate/Public/Widgets/Docking/SDockTab.h"
#include "Runtime/Slate/Public/Widgets/Layout/SSplitter.h"
#include "Runtime/Slate/Public/Widgets/Layout/SScrollBox.h"
#include "Runtime/SlateCore/Public/Widgets/Images/SImage.h"

#include "Runtime/Core/Public/Misc/MessageDialog.h"

#include "Editor/EditorStyle/Public/EditorStyle.h"
#include "Editor/ContentBrowser/Public/ContentBrowserModule.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class UMagicNodeSharpSource;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class SDIFFMainWidget : public SCompoundWidget {
private:
	TWeakObjectPtr<UMagicNodeSharpSource>SourceObject;
	TWeakObjectPtr<UMagicNodeSharpSource>TargetObject;
private:
	TSharedPtr<SBox>DIFF_PANEL;
	TArray<TSharedPtr<SDIFFListItem>>DIFF_SOURCE_ITEMS;
	TArray<TSharedPtr<SDIFFListItem>>DIFF_TARGET_ITEMS;
	//
	TSharedPtr<SButton>DIFF_Button_PICK_SOURCE;
	TSharedPtr<SButton>DIFF_Button_PICK_TARGET;
	TSharedPtr<SButton>DIFF_Button_PICK_ITEM;
	TSharedPtr<SButton>DIFF_Button_CLEAR_ALL;
	TSharedPtr<SButton>DIFF_Button_APPLY;
	//
	TSharedPtr<SScrollBox>DIFF_SOURCE_LIST;
	TSharedPtr<SScrollBox>DIFF_TARGET_LIST;
public:
	SLATE_BEGIN_ARGS(SDIFFMainWidget)
	{}
	SLATE_END_ARGS()
	//
	//
	void Construct(const FArguments &InArgs);
	//
	//
	void RebuildListViews(UMagicNodeSharpSource* Source, UMagicNodeSharpSource* Target);
	void RebuildSourceListView();
	void RebuildTargetListView();
	//
	FText GetSourceName() const;
	FText GetTargetName() const;
	int32 FindLastPickedSource() const;
	int32 FindLastPickedTarget() const;
	int32 FindFirstPickedSource() const;
	int32 FindFirstPickedTarget() const;
	//
	TSharedPtr<SDIFFListItem>FindTargetWidget(const FString &Line);
	TSharedPtr<SDIFFListItem>FindSourceWidget(const FString &Line);
	//
	void OnSourceScrolled(float Offset);
	void OnTargetScrolled(float Offset);
	//
	FReply OnClickedApply();
	FReply OnClickedSourceListItem(const FName Item);
	FReply OnClickedTargetListItem(const FName Item);
	//
	bool IsScriptCompare() const;
	//
	//
	virtual bool IsInteractable() const override;
	virtual bool SupportsKeyboardFocus() const override;
	//
	virtual FReply OnKeyUp(const FGeometry &MyGeometry, const FKeyEvent &InKeyEvent) override;
	virtual FReply OnKeyDown(const FGeometry &MyGeometry, const FKeyEvent &InKeyEvent) override;
public:
	static EDIFF_Mod Modifier;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////