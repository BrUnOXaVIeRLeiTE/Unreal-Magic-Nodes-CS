//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
///			Copyright 2021 (C) Bruno Xavier B. Leite
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "CS_EditorStyle.h"
#include "MagicNodeSharpEditor_Shared.h"

#include "Styling/SlateStyle.h"
#include "Interfaces/IPluginManager.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define PLUGIN_BRUSH(RelativePath,...) FSlateImageBrush(FMagicNodeSharpEditorStyle::InContent(RelativePath,TEXT(".png")),__VA_ARGS__)

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TSharedPtr<FSlateStyleSet>FMagicNodeSharpEditorStyle::StyleSet = nullptr;
TSharedPtr<ISlateStyle>FMagicNodeSharpEditorStyle::Get() {return StyleSet;}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

FString FMagicNodeSharpEditorStyle::InContent(const FString &RelativePath, const TCHAR* Extension) {
	static FString Content = IPluginManager::Get().FindPlugin(CS_PLUGIN_NAME)->GetContentDir();
	return (Content/RelativePath)+Extension;
}

FName FMagicNodeSharpEditorStyle::GetStyleSetName() {
	static FName StyleName(TEXT("MagicNodeSharpEditorStyle"));
	return StyleName;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void FMagicNodeSharpEditorStyle::Initialize() {
	if (StyleSet.IsValid()) {return;}
	//
	const FVector2D Icon16x16(16.f,16.f);
	const FVector2D Icon20x20(20.f,20.f);
	const FVector2D Icon40x40(40.f,40.f);
	const FVector2D Icon128x128(128.f,128.f);
	//
	StyleSet = MakeShareable(new FSlateStyleSet(GetStyleSetName()));
	StyleSet->SetContentRoot(IPluginManager::Get().FindPlugin(CS_PLUGIN_NAME)->GetContentDir());
	//
	StyleSet->Set("ClassIcon.MagicNodeSharpSource", new PLUGIN_BRUSH(TEXT("Icons/MagicNodeSharp_16x"),Icon16x16));
	StyleSet->Set("ClassThumbnail.MagicNodeSharpSource", new PLUGIN_BRUSH(TEXT("Icons/MagicNodeSharp_128x"),Icon128x128));
	//
	StyleSet->Set("SourceView.FolderClosed", new PLUGIN_BRUSH(TEXT("Icons/FolderClosed_16x"),Icon16x16));
	StyleSet->Set("SourceView.ConfigFile", new PLUGIN_BRUSH(TEXT("Icons/ConfigFile_16x"),Icon16x16));
	StyleSet->Set("SourceView.Script", new PLUGIN_BRUSH(TEXT("Icons/CS_Script_20x"),Icon16x16));
	StyleSet->Set("SourceView.CsApp", new PLUGIN_BRUSH(TEXT("Icons/CS_CSApp_40x"),Icon16x16));
	StyleSet->Set("SourceView.Engine", new PLUGIN_BRUSH(TEXT("Icons/Engine_16x"),Icon16x16));
	StyleSet->Set("SourceView.Text", new PLUGIN_BRUSH(TEXT("Icons/Script_16x"),Icon16x16));
	//
	StyleSet->Set("MagicNodeSharpEditor.Save", new PLUGIN_BRUSH(TEXT("UI/KCS_Save_40x"),Icon40x40));
	StyleSet->Set("MagicNodeSharpEditor.Save.Small", new PLUGIN_BRUSH(TEXT("UI/KCS_Save_40x"),Icon20x20));
	//
	StyleSet->Set("MagicNodeSharpEditor.Compile", new PLUGIN_BRUSH(TEXT("UI/KCS_Compile_40x"),Icon40x40));
	StyleSet->Set("MagicNodeSharpEditor.Compile.Small", new PLUGIN_BRUSH(TEXT("UI/KCS_Compile_40x"),Icon20x20));
	//
	StyleSet->Set("MagicNodeSharpEditor.VSCode", new PLUGIN_BRUSH(TEXT("UI/KCS_VSCode_40x"),Icon40x40));
	StyleSet->Set("MagicNodeSharpEditor.VSCode.Small", new PLUGIN_BRUSH(TEXT("UI/KCS_VSCode_40x"),Icon20x20));
	//
	StyleSet->Set("MagicNodeSharpEditor.VSGen", new PLUGIN_BRUSH(TEXT("UI/KCS_VSSln_40x"),Icon40x40));
	StyleSet->Set("MagicNodeSharpEditor.VSGen.Small", new PLUGIN_BRUSH(TEXT("UI/KCS_VSSln_40x"),Icon20x20));
	//
	StyleSet->Set("MagicNodeSharpEditor.VSLaunch", new PLUGIN_BRUSH(TEXT("UI/KCS_VSLaunch_40x"),Icon40x40));
	StyleSet->Set("MagicNodeSharpEditor.VSLaunch.Small", new PLUGIN_BRUSH(TEXT("UI/KCS_VSLaunch_40x"),Icon20x20));
	//
	StyleSet->Set("MagicNodeSharpEditor.DiffTool", new PLUGIN_BRUSH(TEXT("UI/KCS_Diff_40x"),Icon40x40));
	StyleSet->Set("MagicNodeSharpEditor.DiffTool.Small", new PLUGIN_BRUSH(TEXT("UI/KCS_Diff_40x"),Icon20x20));
	//
	StyleSet->Set("MagicNodeSharpEditor.Help", new PLUGIN_BRUSH(TEXT("UI/KCS_Help_40x"),Icon40x40));
	StyleSet->Set("MagicNodeSharpEditor.Help.Small", new PLUGIN_BRUSH(TEXT("UI/KCS_Help_40x"),Icon20x20));
	//
	StyleSet->Set("CS.Error", new PLUGIN_BRUSH(TEXT("UI/KCS_Error_16x"),Icon16x16));
	StyleSet->Set("CS.Success", new PLUGIN_BRUSH(TEXT("UI/KCS_Success_16x"),Icon16x16));
	StyleSet->Set("CS.Warning", new PLUGIN_BRUSH(TEXT("UI/KCS_Warning_16x"),Icon16x16));
	//
	FSlateStyleRegistry::RegisterSlateStyle(*StyleSet.Get());
};

void FMagicNodeSharpEditorStyle::Shutdown() {
	if (StyleSet.IsValid()) {
		FSlateStyleRegistry::UnRegisterSlateStyle(*StyleSet.Get());
		ensure(StyleSet.IsUnique()); StyleSet.Reset();
	}///
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#undef PLUGIN_BRUSH

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////