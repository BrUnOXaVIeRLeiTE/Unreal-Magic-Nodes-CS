/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
///			Copyright 2021 (C) Bruno Xavier B. Leite
//////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Runtime/Slate/Public/Widgets/Layout/SBox.h"
#include "Runtime/Slate/Public/Widgets/Layout/SBorder.h"
#include "Runtime/Slate/Public/Framework/SlateDelegates.h"
#include "Runtime/SlateCore/Public/Widgets/Images/SImage.h"

#include "Editor/EditorStyle/Public/EditorStyle.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum class EDIFF_Owner : uint8 {
	Source,
	Target
};

enum class EDIFF_State : uint8 {
	Deleted,
	NewLine,
	Unchanged,
	PickedDeleted,
	PickedNewLine
};

enum class EDIFF_Mod : uint8 {
	Normal,
	Shift,
	Ctrl,
	Alt
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class SDIFFListItem : public SCompoundWidget {
private:
	FString Line;
	EDIFF_Owner Owner;
	EDIFF_State State;
	//
	TSharedPtr<SBorder>Button;
protected:
	const FButtonStyle* DeletedStyle;
	const FButtonStyle* NewLineStyle;
	const FButtonStyle* UnchangedStyle;
	const FButtonStyle* PickedDeletedStyle;
	const FButtonStyle* PickedNewLineStyle;
	//
	FOnClicked OnClicked;
public:
	SDIFFListItem();
	virtual ~SDIFFListItem() override;
	//
	//
	SLATE_BEGIN_ARGS(SDIFFListItem)
		: _Line(FString())
		, _Owner(EDIFF_Owner::Source)
		, _State(EDIFF_State::Unchanged)
	{}//
		SLATE_ATTRIBUTE(FString,Line)
		SLATE_ATTRIBUTE(EDIFF_Owner,Owner)
		SLATE_ATTRIBUTE(EDIFF_State,State)
		//
		SLATE_EVENT(FOnClicked,OnClicked)
	SLATE_END_ARGS()
	//
	//
	void SetButtonStyle();
	void SetLine(const FString &New);
	void SetState(const EDIFF_State InState);
	void SetOwner(const EDIFF_Owner InOwner);
	//
	const EDIFF_State GetState() const;
	const EDIFF_Owner GetOwner() const;
	const FString &GetLine() const;
	//
	FText GetCaption() const;
	//
	//
	void Construct(const FArguments &InArgs);
	//
	virtual bool IsInteractable() const override;
	virtual bool SupportsKeyboardFocus() const override;
	//
	virtual void OnMouseLeave(const FPointerEvent &MouseEvent) override;
	virtual void OnMouseCaptureLost(const FCaptureLostEvent &CaptureLostEvent) override;
	virtual void OnMouseEnter(const FGeometry &MyGeometry, const FPointerEvent &MouseEvent) override;
	//
	virtual FReply OnMouseButtonUp(const FGeometry &MyGeometry, const FPointerEvent &MouseEvent) override;
	virtual FReply OnMouseButtonDown(const FGeometry &MyGeometry, const FPointerEvent &MouseEvent) override;
	//
	virtual void Tick(const FGeometry &Geometry, const double CurrentTime, const float DeltaTime) override;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////