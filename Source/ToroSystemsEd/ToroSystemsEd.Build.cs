// Copyright (C) RedCraft86. LGPL-3.0-only, see LICENSE file for details.

using UnrealBuildTool;

public class ToroSystemsEd : ModuleRules
{
    public ToroSystemsEd(ReadOnlyTargetRules Target) : base(Target)
    {
	    PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

	    PublicDependencyModuleNames.AddRange([
		    "Core",
		    "CoreUObject",
		    "Engine",
		    "UnrealEd",
		    "ToroEngineEd",
		    "ToroSystems"
	    ]);
    }
}