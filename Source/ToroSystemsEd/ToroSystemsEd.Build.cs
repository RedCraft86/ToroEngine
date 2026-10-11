// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

using UnrealBuildTool;

public class ToroSystemsEd : ModuleRules
{
    public ToroSystemsEd(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange([
	        "Core",
	        "CoreUObject",
	        "UnrealEd",
	        "Engine",
	        "Slate",
	        "SlateCore",
	        "SlateIM",
	        "ToroCoreEd",
	        "ToroSystems"
        ]);
    }
}