// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "SMInstance.h"
#include "ObjectiveGraph.generated.h"

UCLASS(DisplayName = "Objective Root")
class TOROSYSTEMS_API UObjectiveRootNode final : public USMStateMachineInstance
{
	GENERATED_BODY()

public:

	UObjectiveRootNode();
};

UCLASS(Abstract, Blueprintable, BlueprintType)
class TOROSYSTEMS_API UObjectiveGraph final : public USMInstance
{
	GENERATED_BODY()

public:

	UObjectiveGraph();
};
