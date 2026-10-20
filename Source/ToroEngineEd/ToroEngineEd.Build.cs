// Copyright (C) RedCraft86. LGPL-3.0-only, see LICENSE file for details.

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
		    "UnrealEd",
		    "PhysicsCore",
		    "AssetTools",
		    "AssetRegistry",
		    "MeshDescription",
		    "ProceduralMeshComponent",
		    "ToroEngine"
	    ]);
    }
}