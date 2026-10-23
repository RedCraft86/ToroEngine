// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "ToroAssetFactory.h"
#include "DataAssets/ToroDatabase.h"
#include "ToroDatabaseFactory.generated.h"

UCLASS()
class TOROENGINEED_API UToroDatabaseFactory : public UToroAssetFactory
{
	GENERATED_BODY()

public:

	UToroDatabaseFactory()
	{
		bCreateNew = true;
		bUsePicker = true;
		bEditAfterNew = true;
		SupportedClass = UToroDatabase::StaticClass();
	}
};
