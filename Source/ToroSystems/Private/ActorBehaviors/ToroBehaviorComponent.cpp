// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "ActorBehaviors/ToroBehaviorComponent.h"
#include "ActorBehaviors/ToroBehaviorRef.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/UnrealType.h"
#include "GameFramework/Actor.h"
#if WITH_EDITOR
#include "Internationalization/Text.h"
#include "Logging/TokenizedMessage.h"
#include "Misc/DataValidation.h"
#include "Misc/UObjectToken.h"
#include "Containers/Array.h"
#endif

#define LOCTEXT_NAMESPACE "ToroEngine"
UToroBehaviorComponent::UToroBehaviorComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	bAutoActivate = true;
}

void UToroBehaviorComponent::OnRegister()
{
	const AActor* OwnerActor = GetOwner();
	if (!IsTemplate() && IsValid(OwnerActor))
	{
		SetBehaviorRefTypeOwners(OwnerActor);
	}

	Super::OnRegister();
}

void UToroBehaviorComponent::OnUnregister()
{
	SetBehaviorRefTypeOwners(nullptr);
	Super::OnUnregister();
}

#if WITH_EDITOR
TSubclassOf<UToroBehaviorComponent> UToroBehaviorComponent::DuplicateCheckClass() const
{
	return GetClass();
}

EDataValidationResult UToroBehaviorComponent::IsDataValid(FDataValidationContext& Context) const
{
	const EDataValidationResult SuperResult = Super::IsDataValid(Context);

	const AActor* OwnerActor = GetOwner();
	const TSubclassOf<UToroBehaviorComponent> CheckClass = DuplicateCheckClass();
	if (!IsValid(CheckClass) || !IsValid(OwnerActor))
	{
		return SuperResult;
	}

	TArray<UToroActorComponent*> Components;
	OwnerActor->GetComponents(CheckClass, Components);
	if (Components.ContainsByPredicate([this](const UToroActorComponent* Component) -> bool
	{
		return IsValid(Component) && Component != this;
	}))
	{
		Context.AddMessage(EMessageSeverity::Error)
			->AddToken(FTextToken::Create(FText::Format(LOCTEXT("BehaviorComponentOverLimitP1",
				"Component {0} on actor "), FText::FromString(GetName())
			)))
			->AddToken(FUObjectToken::Create(OwnerActor))
			->AddToken(FTextToken::Create(FText::Format(LOCTEXT("BehaviorComponentOverLimitP2",
				" exceeds the limit. Only one component of class '{0}' (including subclasses) is allowed per actor."),
				FText::FromString(GetNameSafe(CheckClass))
			)));

		return EDataValidationResult::Invalid;
	}

	return SuperResult;
}
#endif

void UToroBehaviorComponent::SetBehaviorRefTypeOwners(const AActor* Owner)
{
	for (TFieldIterator<FStructProperty> It(GetClass(), EFieldIterationFlags::IncludeSuper); It; ++It)
	{
		const FStructProperty* StructProp = *It;
		if (StructProp->Struct == FToroBehaviorRef::StaticStruct())
		{
			if (FToroBehaviorRef* RefPtr = StructProp->ContainerPtrToValuePtr<FToroBehaviorRef>(this, 0))
			{
				RefPtr->BindOwner(Owner);
			}
		}
	}
}
#undef LOCTEXT_NAMESPACE
