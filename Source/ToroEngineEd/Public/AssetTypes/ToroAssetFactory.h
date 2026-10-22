// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "Factories/Factory.h"
#include "ToroAssetFactory.generated.h"

class UToroDataAsset;

UCLASS(Abstract)
class TOROENGINEED_API UToroAssetFactory : public UFactory
{
	GENERATED_BODY()

public:

	UToroAssetFactory();

protected:

	bool bUsePicker;
	FString AssetName;
	TSubclassOf<UToroDataAsset> AssetClass;

	virtual bool ConfigureProperties() override;
	virtual FString GetDefaultNewAssetName() const override;
	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName,
		EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
};
