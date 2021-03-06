//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
///			Copyright 2021 (C) Bruno Xavier B. Leite
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CoreMinimal.h"
#include "Framework/Text/SlateTextRun.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class FWhiteSpaceTextRun : public FSlateTextRun {
private:
	int32 SpacesPerTab;
	float SpaceWidth;
	float TabWidth;
protected:
	FWhiteSpaceTextRun(const FRunInfo &InRunInfo, const TSharedRef<const FString>&InText, const FTextBlockStyle &InStyle, const FTextRange &InRange, int32 InSpacesPerTab);
public:
	static TSharedRef<FWhiteSpaceTextRun>Create(const FRunInfo &InRunInfo, const TSharedRef<const FString>&InText, const FTextBlockStyle &InStyle, const FTextRange &InRange, int32 InSpacesPerTab);
	virtual FVector2D Measure(int32 StartIndex, int32 EndIndex, float Scale, const FRunTextContext &TextContext) const override;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////