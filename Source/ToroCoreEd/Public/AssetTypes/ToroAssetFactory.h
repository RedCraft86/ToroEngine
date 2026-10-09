// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#pragma once

#include "Factories/Factory.h"
#include "ToroAssetFactory.generated.h"

class UToroDataAsset;

/**
 * Base factory for transactional UToroDataAsset instances, opened for editing after creation.
 * Derived factories must supply a concrete SupportedClass or enable class selection beneath
 * a supported base. Invalid, abstract, deprecated, superseded, and incompatible classes are rejected.
 */
UCLASS(Abstract)
class TOROCOREED_API UToroAssetFactory : public UFactory
{
	GENERATED_BODY()

public:

	UToroAssetFactory();

protected:

	/** Opens a native-class picker beneath SupportedClass during configuration when enabled. */
	bool bUsePicker;

	/** Optional default asset name, sanitized as a filename; names shorter than two characters receive a New prefix. */
	FString AssetName;

	/** Class selected during configuration; cleared before each attempt and used in preference to the creation argument. */
	TSubclassOf<UToroDataAsset> AssetClass;

	virtual bool ConfigureProperties() override;
	virtual FString GetDefaultNewAssetName() const override;
	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName,
		EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
};
