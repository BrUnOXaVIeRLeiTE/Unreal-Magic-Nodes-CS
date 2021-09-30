/// Copyright 2021 (C) Bruno Xavier B. Leite

using UnrealBuildTool;
using System.IO;

public class MagicNodeSharpEditor : ModuleRules {
	public MagicNodeSharpEditor(ReadOnlyTargetRules Target) : base(Target) {
		PrivatePCHHeaderFile = "Public/MagicNodeSharpEditor_Shared.h";
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		bEnforceIWYU = true;
		//
		PublicDependencyModuleNames.AddRange(
			new string[] {
				"Core",
				"Engine",
				"CoreUObject",
				"MagicNodeSharp",
				"MagicNodeSharpKismet"
			}///
		);//
		//
		PrivateDependencyModuleNames.AddRange(
			new string[] {
				"Slate",
				"Kismet",
				"UnrealEd",
				"Projects",
				"InputCore",
				"SlateCore",
				"MessageLog",
				"AssetTools",
				"GraphEditor",
				"LevelEditor",
				"EditorStyle",
				"KismetWidgets",
				"ContentBrowser",
				"PropertyEditor",
				"KismetCompiler",
				"BlueprintGraph",
				"DirectoryWatcher"
			}///
		);//
		//
		PublicIncludePaths.Add(Path.Combine(ModuleDirectory,"Public"));
		PublicIncludePaths.Add(Path.Combine(ModuleDirectory,"Classes"));
		PrivateIncludePaths.Add(Path.Combine(ModuleDirectory,"Private"));
	}///
}