// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#include "DetailsPanel/ToroStructCustomization.h"
#include "GameplayTagContainer.h"
#include "ToroCoreEd.h"

void FToroStructCustomization::ForwardMetadata(const TSharedPtr<IPropertyHandle>& Property) const
{
	const TSharedPtr<IPropertyHandle> StructHandle = WeakStructHandle.Pin();
	if (!Property.IsValid() || !StructHandle.IsValid() || !Property->IsValidHandle() || !StructHandle->IsValidHandle())
	{
		return;
	}

	if (const FStructProperty* InnerStruct = CastField<FStructProperty>(Property->GetProperty()))
	{
		if (InnerStruct->Struct == FGameplayTag::StaticStruct())
		{
			UE_LOG(LogToroCoreEd, Error,
				TEXT("ForwardMetadata is unsupported for FGameplayTag property %s in %s. Use SGameplayTagCombo to construct your own."),
				*Property->GetPropertyDisplayName().ToString(), *StructHandle->GetPropertyDisplayName().ToString()
			);
			return;
		}
	}

	if (const FProperty* StructProp = StructHandle->GetProperty())
	{
		if (const TMap<FName, FString>* MetadataMap = StructProp->GetMetaDataMap())
		{
			for (const TPair<FName, FString>& Metadata : *MetadataMap)
			{
				Property->SetInstanceMetaData(Metadata.Key, Metadata.Value);
			}
		}
	}
}

void FToroStructCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> StructHandle,
	FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	WeakStructHandle = StructHandle;
}

void FToroStructCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> StructHandle,
	IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	WeakStructHandle = StructHandle;
}
