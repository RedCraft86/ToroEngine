// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "ToroAssetDefinition.h"
#include "DataAssets/ToroDatabase.h"
#include "ToroDatabaseDefinition.generated.h"

UCLASS()
class TOROENGINEED_API UToroDatabaseDefinition : public UToroAssetDefinition
{
	GENERATED_BODY()

public:

	virtual TSoftClassPtr<UObject> GetAssetClass() const override
	{
		return UToroDatabase::StaticClass();
	}

	virtual FText GetAssetDisplayName() const override
	{
		return NSLOCTEXT("ToroEngine", "ToroDatabaseLabel", "Database");
	}
};
