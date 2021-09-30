/// Copyright 2021 (C) Bruno Xavier B. Leite

using UnrealBuildTool;
using System.IO;

public class MagicNodeSharpKismet : ModuleRules {
    public MagicNodeSharpKismet(ReadOnlyTargetRules Target) : base(Target) {
		PrivatePCHHeaderFile = "Public/MagicNodeSharpKismet_Shared.h";
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		bEnforceIWYU = true;
		//
		PublicDependencyModuleNames.AddRange(
			new string[] {
				"Core",
				"Json",
				"Engine",
				"CoreUObject",
				"MagicNodeSharp",
				"JsonUtilities"
            }///
        );//
        //
        PublicDependencyModuleNames.AddRange(
            new string[] {
				"Slate",
				"Kismet",
				"UnrealEd",
				"Projects",
				"InputCore",
				"SlateCore",
				"AssetTools",
				"GraphEditor",
				"LevelEditor",
				"EditorStyle",
				"KismetWidgets",
				"ContentBrowser",
				"PropertyEditor",
				"KismetCompiler",
				"BlueprintGraph"
            }///
        );//
        //
        PublicIncludePaths.Add(Path.Combine(ModuleDirectory,"Public"));
        PublicIncludePaths.Add(Path.Combine(ModuleDirectory,"Classes"));
        PrivateIncludePaths.Add(Path.Combine(ModuleDirectory,"Private"));
    }///
}