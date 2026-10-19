// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#include "GameEntity/GameEntityComponent.h"
#include "GameEntity/GameEntityComponentRef.h"
#include "UObject/UnrealType.h"
#if WITH_EDITOR
#include "Misc/DataValidation.h"
#include "Misc/UObjectToken.h"
#endif

UGameEntityComponent::UGameEntityComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	bAutoActivate = true;

#if WITH_EDITORONLY_DATA
	MaxInstancesPerActor = 1;
#endif
}

void UGameEntityComponent::OnRegister()
{
	// Fix: Bind references before the base invokes the Blueprint construction event.
	if (!IsTemplate() && IsValid(GetOwner()))
	{
		SetReferenceOwner(GetOwner());
	}
	Super::OnRegister();
}

void UGameEntityComponent::OnUnregister()
{
	SetReferenceOwner(nullptr);
	Super::OnUnregister();
}

#if WITH_EDITOR
EDataValidationResult UGameEntityComponent::IsDataValid(FDataValidationContext& Context) const
{
	return Super::IsDataValid(Context);
}
#endif

void UGameEntityComponent::SetReferenceOwner(AActor* Owner)
{
	for (TFieldIterator<FStructProperty> It(GetClass(), EFieldIteratorFlags::IncludeSuper); It; ++It)
	{
		const FStructProperty* StructProp = *It;
		if (StructProp->Struct == FGameEntityComponentRef::StaticStruct())
		{
			if (FGameEntityComponentRef* RefPtr = StructProp->ContainerPtrToValuePtr<FGameEntityComponentRef>(this, 0))
			{
				RefPtr->BindOwner(Owner);
			}
		}
	}
}
