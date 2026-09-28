// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#include "Components/ToroActorComponent.h"
#if WITH_EDITOR
#include "Misc/DataValidation.h"
#include "Misc/UObjectToken.h"
#endif

UToroActorComponent::UToroActorComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UToroActorComponent::OnConstruction()
{
	EventConstruction();
}

void UToroActorComponent::OnRegister()
{
#if WITH_EDITORONLY_DATA
	bTickInEditor = bTickDuringEditor;
#endif
	Super::OnRegister();
	OnConstruction();
}

#if WITH_EDITOR
bool UToroActorComponent::CanEditChange(const FProperty* InProperty) const
{
	if (InProperty && InProperty->GetFName() == GET_MEMBER_NAME_CHECKED(UToroActorComponent, MaxInstancesPerActor))
	{
		return IsTemplate();
	}

	return Super::CanEditChange(InProperty);
}

void UToroActorComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (!IsTemplate())
	{
		OnConstruction();
	}
}

EDataValidationResult UToroActorComponent::IsDataValid(FDataValidationContext& Context) const
{
	const AActor* OwnerActor = GetOwner();
	if (MaxInstancesPerActor > 0 && IsValid(OwnerActor))
	{
		TArray<UToroActorComponent*> Components;
		OwnerActor->GetComponents(GetClass(), Components);
		const int32 InstanceIdx = Components.IndexOfByKey(this);
		if (InstanceIdx >= MaxInstancesPerActor)
		{
			Context.AddMessage(FMessageLog("MapCheck").Error()
				->AddToken(FUObjectToken::Create(OwnerActor))
				->AddToken(FTextToken::Create(FText::Format(
					NSLOCTEXT("ToroEngine", "Validate_Message_ComponentOverLimit", "{0}::{1} component class exceeds limit ({2}/{3})"),
					FText::FromString(OwnerActor->GetClass()->GetName()), FText::FromString(GetName()), InstanceIdx + 1, MaxInstancesPerActor
				))));
			return EDataValidationResult::Invalid;
		}
	}

	return Super::IsDataValid(Context);
}
#endif