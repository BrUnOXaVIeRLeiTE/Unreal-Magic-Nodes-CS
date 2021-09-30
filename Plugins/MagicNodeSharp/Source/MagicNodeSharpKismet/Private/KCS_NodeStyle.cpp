//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
///			Copyright 2021 (C) Bruno Xavier B. Leite
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "KCS_NodeStyle.h"
#include "MagicNodeSharpKismet_Shared.h"

#include "Fonts/CompositeFont.h"

#include "Styling/CoreStyle.h"
#include "Styling/SlateStyle.h"
#include "Styling/SlateTypes.h"
#include "Styling/SlateStyleRegistry.h"

#include "Interfaces/IPluginManager.h"


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define MTF_FONT(Name,RelativePath,...) FCompositeFont(Name,FKCS_NodeStyle::InContent(RelativePath,TEXT(".ttf")),EFontHinting::Default,EFontLoadingPolicy::LazyLoad);
#define IMAGE_BRUSH(RelativePath,...) FSlateImageBrush(FKCS_NodeStyle::InContent(RelativePath,TEXT(".png")),__VA_ARGS__)
#define BOX_BRUSH(RelativePath,...) FSlateBoxBrush(FKCS_NodeStyle::InContent(RelativePath,TEXT(".png") ),__VA_ARGS__)
#define TTF_FONT(RelativePath,...) FSlateFontInfo(FKCS_NodeStyle::InContent(RelativePath,TEXT(".ttf")),__VA_ARGS__)

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TSharedPtr<FSlateStyleSet>FKCS_NodeStyle::StyleSet = nullptr;
TSharedPtr<ISlateStyle>FKCS_NodeStyle::Get() {return StyleSet;}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

FString FKCS_NodeStyle::InContent(const FString &RelativePath, const TCHAR* Extension) {
	static FString Content = IPluginManager::Get().FindPlugin(CS_PLUGIN_NAME)->GetContentDir();
	return (Content/RelativePath)+Extension;
}

FName FKCS_NodeStyle::GetStyleSetName() {
	static FName StyleName(TEXT("KCS_NodeStyle"));
	return StyleName;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void FKCS_NodeStyle::Initialize() {
	if (StyleSet.IsValid()) {return;}
	//
	StyleSet = MakeShareable(new FSlateStyleSet(GetStyleSetName()));
	StyleSet->SetContentRoot(IPluginManager::Get().FindPlugin(CS_PLUGIN_NAME)->GetContentDir());
	//
	//
	const FVector2D Icon8x8(8.f,8.f);
	const FVector2D Icon16x16(16.f,16.f);
	const FVector2D Icon20x20(22.f,22.f);
	const FVector2D Icon32x32(32.f,32.f);
	const FVector2D Icon40x40(40.f,40.f);
	const FVector2D Icon48x48(48.f,48.f);
	const FVector2D Icon64x64(64.f,64.f);
	const FVector2D Icon128x128(128.f,128.f);
	//
	FSlateFontInfo TTF_Hack = TTF_FONT(TEXT("UI/Hack"),14);
	//
	const FTextBlockStyle TBS_CodeStyle = FTextBlockStyle()
	.SetHighlightShape(BOX_BRUSH("UI/KCS_TextHighlight",FMargin(3.f/8.f)))
	.SetHighlightColor(FLinearColor(FColor(255,175,5,255)))
	.SetShadowColorAndOpacity(FLinearColor::Black)
	.SetColorAndOpacity(FLinearColor::White)
	.SetShadowOffset(FVector2D::ZeroVector)
	.SetFont(TTF_Hack);
	//
	//
	/// Icons:
	{
		StyleSet->Set("KCS.MagicNodeSharpIcon", new IMAGE_BRUSH(TEXT("Icons/MagicNodeSharp_128x"),Icon128x128));
		StyleSet->Set("KCS.MagicNodeSharpIcon.Small", new IMAGE_BRUSH(TEXT("Icons/MagicNodeSharp_128x"),Icon20x20));
		//
		StyleSet->Set("KCS.Toolbar.SaveScript", new IMAGE_BRUSH(TEXT("UI/KCS_Save_40x"),Icon40x40));
		StyleSet->Set("KCS.Toolbar.ReloadScript", new IMAGE_BRUSH(TEXT("UI/KCS_Reload_40x"),Icon40x40));
		StyleSet->Set("KCS.Toolbar.CompileScript", new IMAGE_BRUSH(TEXT("UI/KCS_Compile_40x"),Icon40x40));
		//
		StyleSet->Set("KCS.Toolbar.SaveScript.Small", new IMAGE_BRUSH(TEXT("UI/KCS_Save_40x"),Icon20x20));
		StyleSet->Set("KCS.Toolbar.ReloadScript.Small", new IMAGE_BRUSH(TEXT("UI/KCS_Reload_40x"),Icon20x20));
		StyleSet->Set("KCS.Toolbar.CompileScript.Small", new IMAGE_BRUSH(TEXT("UI/KCS_Compile_40x"),Icon20x20));
		//
		StyleSet->Set("KCS.Static", new IMAGE_BRUSH(TEXT("UI/KCS_Static_16x"),Icon16x16));
		StyleSet->Set("KCS.Virtual", new IMAGE_BRUSH(TEXT("UI/KCS_Virtual_16x"),Icon16x16));
		StyleSet->Set("KCS.Namespace", new IMAGE_BRUSH(TEXT("UI/KCS_Namespace_16x"),Icon16x16));
		//
		StyleSet->Set("KCS.Class.Public", new IMAGE_BRUSH(TEXT("UI/KCS_Class_16x"),Icon16x16));
		StyleSet->Set("KCS.Class.Private", new IMAGE_BRUSH(TEXT("UI/KCS_ClassPrivate_16x"),Icon16x16));
		StyleSet->Set("KCS.Class.Protected", new IMAGE_BRUSH(TEXT("UI/KCS_ClassProtected_16x"),Icon16x16));
		//
		StyleSet->Set("KCS.Field.Public", new IMAGE_BRUSH(TEXT("UI/KCS_Field_16x"),Icon16x16));
		StyleSet->Set("KCS.Field.Private", new IMAGE_BRUSH(TEXT("UI/KCS_FieldPrivate_16x"),Icon16x16));
		StyleSet->Set("KCS.Field.Protected", new IMAGE_BRUSH(TEXT("UI/KCS_FieldProtected_16x"),Icon16x16));
		//
		StyleSet->Set("KCS.Property.Public", new IMAGE_BRUSH(TEXT("UI/KCS_Property_16x"),Icon16x16));
		StyleSet->Set("KCS.Property.Private", new IMAGE_BRUSH(TEXT("UI/KCS_PropertyPrivate_16x"),Icon16x16));
		StyleSet->Set("KCS.Property.Protected", new IMAGE_BRUSH(TEXT("UI/KCS_PropertyProtect_16x"),Icon16x16));
		//
		StyleSet->Set("KCS.Function.Public", new IMAGE_BRUSH(TEXT("UI/KCS_Method_16x"),Icon16x16));
		StyleSet->Set("KCS.Function.Private", new IMAGE_BRUSH(TEXT("UI/KCS_MethodPrivate_16x"),Icon16x16));
		StyleSet->Set("KCS.Function.Protected", new IMAGE_BRUSH(TEXT("UI/KCS_MethodProtect_16x"),Icon16x16));
		//
		StyleSet->Set("KCS.Error", new IMAGE_BRUSH(TEXT("UI/KCS_Error_16x"),Icon16x16));
		StyleSet->Set("KCS.Success", new IMAGE_BRUSH(TEXT("UI/KCS_Success_16x"),Icon16x16));
		StyleSet->Set("KCS.Warning", new IMAGE_BRUSH(TEXT("UI/KCS_Warning_16x"),Icon16x16));
		//
		StyleSet->Set("KCS.Lines", new IMAGE_BRUSH("UI/KCS_TextHighlight",Icon8x8));
		StyleSet->Set("KCS.Focus", new IMAGE_BRUSH("UI/KCS_LineFocus_40x",Icon16x16));
	}
	//
	/// Text Editor:
	{
		StyleSet->Set("KCS.CodeBlockStyle",TBS_CodeStyle);
		//
		StyleSet->Set("KCS.SyntaxHighlight.Processor",FTextBlockStyle(TBS_CodeStyle).SetColorAndOpacity(FLinearColor(FColor(125,125,125,255))));
		StyleSet->Set("KCS.SyntaxHighlight.Container",FTextBlockStyle(TBS_CodeStyle).SetColorAndOpacity(FLinearColor(FColor(125,235,185,255))));
		StyleSet->Set("KCS.SyntaxHighlight.Property",FTextBlockStyle(TBS_CodeStyle).SetColorAndOpacity(FLinearColor(FColor(205,225,250,255))));
		StyleSet->Set("KCS.SyntaxHighlight.Operator",FTextBlockStyle(TBS_CodeStyle).SetColorAndOpacity(FLinearColor(FColor(95,255,245,255))));
		StyleSet->Set("KCS.SyntaxHighlight.Normal",FTextBlockStyle(TBS_CodeStyle).SetColorAndOpacity(FLinearColor(FColor(255,245,205,255))));
		StyleSet->Set("KCS.SyntaxHighlight.Keyword",FTextBlockStyle(TBS_CodeStyle).SetColorAndOpacity(FLinearColor(FColor(50,125,255,255))));
		StyleSet->Set("KCS.SyntaxHighlight.String",FTextBlockStyle(TBS_CodeStyle).SetColorAndOpacity(FLinearColor(FColor(255,155,135,255))));
		StyleSet->Set("KCS.SyntaxHighlight.Function",FTextBlockStyle(TBS_CodeStyle).SetColorAndOpacity(FLinearColor(FColor(255,105,5,255))));
		StyleSet->Set("KCS.SyntaxHighlight.Number",FTextBlockStyle(TBS_CodeStyle).SetColorAndOpacity(FLinearColor(FColor(55,255,155,255))));
		StyleSet->Set("KCS.SyntaxHighlight.Comment",FTextBlockStyle(TBS_CodeStyle).SetColorAndOpacity(FLinearColor(FColor(55,125,65,255))));
		StyleSet->Set("KCS.SyntaxHighlight.Event",FTextBlockStyle(TBS_CodeStyle).SetColorAndOpacity(FLinearColor(FColor(125,165,235,255))));
		StyleSet->Set("KCS.SyntaxHighlight.Class",FTextBlockStyle(TBS_CodeStyle).SetColorAndOpacity(FLinearColor(FColor(75,185,245,255))));
		StyleSet->Set("KCS.SyntaxHighlight.Macro",FTextBlockStyle(TBS_CodeStyle).SetColorAndOpacity(FLinearColor(FColor(225,10,155,255))));
		StyleSet->Set("KCS.SyntaxHighlight.Illegal",FTextBlockStyle(TBS_CodeStyle).SetColorAndOpacity(FLinearColor(FColor(175,5,5,255))));
		StyleSet->Set("KCS.SyntaxHighlight.Type",FTextBlockStyle(TBS_CodeStyle).SetColorAndOpacity(FLinearColor(FColor(255,175,5,255))));
	}
	//
	//
	FSlateStyleRegistry::RegisterSlateStyle(*StyleSet.Get());
};

void FKCS_NodeStyle::Shutdown() {
	if (StyleSet.IsValid()) {
		FSlateStyleRegistry::UnRegisterSlateStyle(*StyleSet.Get());
		ensure(StyleSet.IsUnique()); StyleSet.Reset();
	}///
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#undef TTF_FONT
#undef MTF_FONT
#undef BOX_BRUSH
#undef IMAGE_BRUSH

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////