// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

using UnrealBuildTool;

public class ToroEngineEd : ModuleRules
{
	public ToroEngineEd(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange([
			"Core",
			"CoreUObject",
			"Engine",
			"Slate",
			"SlateCore",
			"SlateIM",
			"UnrealEd",
			"ToolMenus",
			"ToroCoreEd",
			"ToroEngine"
		]);
	}
}