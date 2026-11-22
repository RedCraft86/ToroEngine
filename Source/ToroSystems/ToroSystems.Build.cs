// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

using UnrealBuildTool;

public class ToroSystems : ModuleRules
{
	public ToroSystems(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange([
			"Core",
			"CoreUObject",
			"Engine",
			"PhysicsCore",
			"Slate",
			"SlateCore",
			"UMG",
			"Json",
			"RenderCore",
			"EngineSettings",
			"MetasoundEngine",
			"AudioModulation",
			"AsyncMessageSystem",
			"DeveloperSettings",
			"LevelSequence",
			"EngineCameras",
			"EnhancedInput",
			"GameplayTags",
			"CommonUI",
			"CommonInput",
			"ToroEngine",
			"UE5Coro"
		]);

		if (Target.Type == TargetType.Editor)
		{
			PrivateDependencyModuleNames.AddRange([
				"UnrealEd"
			]);
		}
	}
}
