/// Copyright 2021 (C) Bruno Xavier B. Leite

using UnrealBuildTool;
using System.IO;

public class MonoRuntime : ModuleRules {
	public MonoRuntime(ReadOnlyTargetRules Target) : base(Target) {
		Type = ModuleType.External;
		//
		PublicIncludePaths.Add(Path.Combine(ModuleDirectory,"Include"));
		//
		string DIRxCLR = Path.Combine(ModuleDirectory,"../../Binaries");
		string DIRxLIB = Path.Combine(ModuleDirectory,"../../Binaries/Mono/lib/mono");
		//
		if (Target.Platform==UnrealTargetPlatform.Win64) {
			RuntimeDependencies.Add(Path.Combine(DIRxCLR,"MagicNode.dll"));
			RuntimeDependencies.Add(Path.Combine(DIRxCLR,"UnrealEngine.dll"));
			//
			RuntimeDependencies.Add(Path.Combine(DIRxCLR,"mono-2.0-sgen.dll"));
			//
			RuntimeDependencies.Add(Path.Combine(DIRxLIB,"4.0","mscorlib.dll"));
			RuntimeDependencies.Add(Path.Combine(DIRxLIB,"4.8-api","*.dll"));
			RuntimeDependencies.Add(Path.Combine(DIRxLIB,"4.5","*.dll"));
			//
			RuntimeDependencies.Add("$(ProjectDir)/Binaries/Managed/*.dll");
		}///
	}
}