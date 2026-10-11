// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

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
		    "Slate",
		    "SlateCore",
		    "ToroCore",
		    "ToroEngine"
	    ]);
    }
}