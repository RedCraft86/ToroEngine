// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#include "AssetTypes/ToroAssetFactory.h"
#include "AssetTypes/ToroAssetClassFilter.h"
#include "Kismet2/SClassPickerDialog.h"
#include "Objects/ToroDataAsset.h"
#include "ClassViewerModule.h"
#include "Misc/Paths.h"

UToroAssetFactory::UToroAssetFactory()
{
	bCreateNew = true;
	bUsePicker = false;
	bEditAfterNew = true;
	SupportedClass = UToroDataAsset::StaticClass();
}

bool UToroAssetFactory::ConfigureProperties()
{
	AssetClass = nullptr;
	if (!IsValid(SupportedClass) || !SupportedClass->IsChildOf(UToroDataAsset::StaticClass()))
	{
		return false;
	}

	if (bUsePicker)
	{
		// Ensure the class viewer module is available before opening the picker.
		FModuleManager::LoadModuleChecked<FClassViewerModule>(TEXT("ClassViewer"));

		FClassViewerInitializationOptions Options;
		Options.Mode = EClassViewerMode::ClassPicker;
		Options.NameTypeToDisplay = EClassViewerNameTypeToDisplay::DisplayName;

		const TSharedPtr<FToroAssetClassFilter> Filter = MakeShared<FToroAssetClassFilter>(SupportedClass);
		Options.ClassFilters.Add(Filter.ToSharedRef());

		UClass* ChosenClass = nullptr;
		if (!SClassPickerDialog::PickClass(
			NSLOCTEXT("EditorFactories", "CreateDataAssetOptions", "Pick Class For Data Asset Instance"),
			Options, ChosenClass, SupportedClass))
		{
			return false;
		}

		if (!IsValid(ChosenClass) || !ChosenClass->IsChildOf(SupportedClass)
			|| ChosenClass->HasAnyClassFlags(FToroAssetClassFilter::BadClassFlags))
		{
			return false;
		}

		AssetClass = ChosenClass;
		return true;
	}

	if (SupportedClass->HasAnyClassFlags(FToroAssetClassFilter::BadClassFlags))
	{
		return false;
	}

	AssetClass = SupportedClass;
	return true;
}

FString UToroAssetFactory::GetDefaultNewAssetName() const
{
	if (!AssetName.IsEmpty())
	{
		const FString ValidName = FPaths::MakeValidFileName(AssetName);
		return ValidName.Len() < 2 ? TEXT("New") + ValidName : ValidName;
	}

	const UClass* ClassType = AssetClass ? AssetClass.Get() : GetSupportedClass();
	if (ensureMsgf(ClassType, TEXT("Factory %s creating an asset without a valid class."), *GetClass()->GetName()))
	{
		return TEXT("New") + ClassType->GetName();
	}

	return Super::GetDefaultNewAssetName();
}

UObject* UToroAssetFactory::FactoryCreateNew(UClass* InClass, UObject* InParent,
	FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	const UClass* ClassType = AssetClass ? AssetClass.Get() : InClass;
	if (!ensureMsgf(IsValid(ClassType), TEXT("Factory %s creating an asset without a valid class."), *GetClass()->GetName()))
	{
		return nullptr;
	}

	if (!ensureMsgf(ClassType->IsChildOf(UToroDataAsset::StaticClass()),
		TEXT("Factory %s creating an asset that isn't a subtype of UToroDataAsset."), *GetClass()->GetName()))
	{
		return nullptr;
	}

	if (!ensureMsgf(IsValid(SupportedClass) && ClassType->IsChildOf(SupportedClass)
		&& !ClassType->HasAnyClassFlags(FToroAssetClassFilter::BadClassFlags),
		TEXT("Factory %s cannot instantiate the selected asset class."), *GetClass()->GetName()))
	{
		return nullptr;
	}

	return NewObject<UDataAsset>(InParent, ClassType, InName, Flags | RF_Transactional);
}
