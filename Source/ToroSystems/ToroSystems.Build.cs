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
			"InputCore",
			"AsyncMessageSystem",
			"DeveloperSettings",
			"LevelSequence",
			"EnhancedInput",
			"CustomizableSequencerTracks",
			"GameplayTags",
			"CommonUI",
			"ToroEngine",
			"ExpressiveText",
			"SMExtendedRuntime",
			"SMSystem",
			"UE5Coro"
		]);

		PrivateDependencyModuleNames.AddRange([
			"RenderCore",
			"EngineSettings",
			"MetasoundEngine",
			"AudioModulation",
			"MovieScene",
			"EngineCameras",
			"CommonInput"
		]);

		if (Target.Type == TargetType.Editor)
		{
			PrivateDependencyModuleNames.AddRange([
				"UnrealEd"
			]);
		}
	}
}
