// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

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
		    "Projects",
		    "PhysicsCore",
		    "ToolMenus",
		    "AssetTools",
		    "AssetRegistry",
		    "MeshDescription",
		    "AssetDefinition",
		    "ProceduralMeshComponent",
		    "ToroEngine"
	    ]);
    }
}