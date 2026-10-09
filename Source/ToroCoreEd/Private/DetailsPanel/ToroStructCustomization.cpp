// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#include "DetailsPanel/ToroStructCustomization.h"

void FToroStructCustomization::ForwardMetadata(const TSharedPtr<IPropertyHandle>& Property) const
{
	const TSharedPtr<IPropertyHandle> StructHandle = WeakStructHandle.Pin();
	if (!Property.IsValid() || !StructHandle.IsValid() || !Property->IsValidHandle() || !StructHandle->IsValidHandle())
	{
		return;
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
