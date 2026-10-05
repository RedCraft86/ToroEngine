// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

using UnrealBuildTool;

public class ToroCore : ModuleRules
{
    public ToroCore(ReadOnlyTargetRules Target) : base(Target)
    {
	    PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

	    PublicDependencyModuleNames.AddRange(
		    [
			    "Core",
			    "CoreUObject",
			    "Engine",
			    "UMG",
			    "Slate",
			    "SlateCore",
			    "RenderCore",
			    "GameplayTags",
			    "LevelSequence",
			    "MovieScene",
			    "UE5Coro"
		    ]
	    );

	    if (Target.Type == TargetType.Editor)
	    {
		    PrivateDependencyModuleNames.AddRange(
			    [
				    "UnrealEd"
			    ]
		    );
	    }
    }
}