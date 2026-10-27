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
		    "SlateIM",
		    "SlateCore",
		    "PhysicsCore",
		    "UnrealEd",
		    "Projects",
		    "ClassViewer",
		    "ToolMenus",
		    "AssetTools",
		    "AssetRegistry",
		    "PropertyEditor",
		    "MeshDescription",
		    "AssetDefinition",
		    "ProceduralMeshComponent",
		    "GameplayTagsEditor",
		    "StructUtilsEditor",
		    "ToroEngine"
	    ]);
    }
}