/// Copyright 2021 (C) Bruno Xavier B. Leite

using UnrealBuildTool;
using System.IO;

public class MagicNodeSharp : ModuleRules {
	public MagicNodeSharp(ReadOnlyTargetRules Target) : base(Target) {
		PrivatePCHHeaderFile = "Public/MagicNodeSharp_Shared.h";
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		bEnforceIWYU = true;
		//
		PublicDependencyModuleNames.AddRange(
			new string[] {
				"Core",
				"Json",
				"Engine",
				"Projects",
				"CoreUObject",
				"AssetRegistry",
				"JsonUtilities"
			}///
		);//
		//
		PublicDependencyModuleNames.AddRange(
			new string[] {"MonoRuntime"}
		);///
		//
		PublicIncludePaths.Add(Path.Combine(ModuleDirectory,"Public"));
		PublicIncludePaths.Add(Path.Combine(ModuleDirectory,"Classes"));
		PrivateIncludePaths.Add(Path.Combine(ModuleDirectory,"Private"));
		//
		if (Target.Platform!=UnrealTargetPlatform.Win64) {
			PublicDefinitions.Add("MCS_NOT_SUPPORTED=1"); return;
		}///
		//
		string DIRxCLR = Path.GetFullPath(Path.Combine(ModuleDirectory,"../../Binaries"));
		PublicAdditionalLibraries.Add(Path.Combine(DIRxCLR,"Mono/lib/mono-2.0-sgen.lib"));
		PublicDelayLoadDLLs.Add("mono-2.0-sgen.dll");
		//
		if (Target.Type==TargetRules.TargetType.Editor) {
			PublicDependencyModuleNames.AddRange(new string[]{"UnrealEd","DirectoryWatcher"});
			PublicAdditionalLibraries.Add(Path.Combine(DIRxCLR,"Mono/lib/MonoPosixHelper.lib"));
		}///
		//
		///PublicDefinitions.Add("MCS_MARKETPLACE_BUILD=1");
	}///
}