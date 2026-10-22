// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "Factories/Factory.h"
#include "ToroAssetFactory.generated.h"

class UToroDataAsset;

/**
 * Abstract editor factory for creating transactional UToroDataAsset instances.
 * Derived factories configure SupportedClass to a UToroDataAsset subclass.
 * With the picker disabled, SupportedClass must itself be concrete and permitted by the class filter.
 * With the picker enabled, SupportedClass limits the classes offered for selection.
 */
UCLASS(Abstract)
class TOROENGINEED_API UToroAssetFactory : public UFactory
{
	GENERATED_BODY()

public:

	/**
	 * Enables new-asset creation and opening the asset editor after creation.
	 * Defaults to UToroDataAsset as SupportedClass with class selection disabled;
	 * derived factories must configure a usable class or enable the picker.
	 */
	UToroAssetFactory();

protected:

	/**
	 * Whether configuration prompts for a compatible concrete class instead of using SupportedClass directly.
	 * Defaults to false; the picker restricts selection to permitted native subclasses.
	 */
	bool bUsePicker;

	/**
	 * Optional default asset-name override, sanitized for filesystem and Unreal object-name restrictions.
	 * Sanitized names shorter than two characters receive a New prefix.
	 * An empty override uses a name derived from the selected or supported class.
	 */
	FString AssetName;

	/**
	 * Class selected for the current creation attempt, cleared whenever ConfigureProperties() starts.
	 * Successful configuration assigns the picker selection or SupportedClass.
	 * Creation falls back to its InClass argument when no class is selected.
	 */
	TSubclassOf<UToroDataAsset> AssetClass;

	virtual bool ConfigureProperties() override;
	virtual FString GetDefaultNewAssetName() const override;
	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName,
		EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
};
