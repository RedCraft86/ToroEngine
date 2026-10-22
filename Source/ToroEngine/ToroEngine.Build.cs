// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

using UnrealBuildTool;

public class ToroEngine : ModuleRules
{
	public ToroEngine(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange([
			"Core",
			"CoreUObject",
			"Engine",
			"GameplayTags"
		]);

		if (Target.Type == TargetType.Editor)
		{
			PrivateDependencyModuleNames.AddRange([
				"UnrealEd"
			]);
		}
	}
}