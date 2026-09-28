// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#include "Components/ToroSceneComponent.h"
#if WITH_EDITOR
#include "Components/BillboardComponent.h"
#endif

UToroSceneComponent::UToroSceneComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
#if WITH_EDITORONLY_DATA
	bVisualizeComponent = true;
#endif
}

void UToroSceneComponent::OnConstruction()
{
	EventConstruction();
}

void UToroSceneComponent::OnRegister()
{
#if WITH_EDITORONLY_DATA
	bTickInEditor = bTickDuringEditor;
#endif
	Super::OnRegister();
	OnConstruction();
}

#if WITH_EDITOR
void UToroSceneComponent::OnChildAttached(USceneComponent* ChildComponent)
{
	Super::OnChildAttached(ChildComponent);
	CheckForSpriteVisualization();
}

void UToroSceneComponent::OnChildDetached(USceneComponent* ChildComponent)
{
	Super::OnChildDetached(ChildComponent);
	CheckForSpriteVisualization();
}

void UToroSceneComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (!IsTemplate())
	{
		OnConstruction();
	}

	CheckForSpriteVisualization();
}

void UToroSceneComponent::CheckForSpriteVisualization() const
{
	if (!SpriteComponent)
	{
		return;
	}

	if (!bVisualizeComponent)
	{
		SpriteComponent->SetVisibility(false);
		return;
	}

	TArray<USceneComponent*> Children;
	GetChildrenComponents(true, Children);

	SpriteComponent->SetVisibility(true);
	for (USceneComponent* Child : Children)
	{
		if (!IsValid(Child))
		{
			continue;
		}

		const UPrimitiveComponent* Prim = Cast<UPrimitiveComponent>(Child);
		if (Child->bVisualizeComponent || (Prim && Prim->IsVisible() && !Prim->IsVisualizationComponent()))
		{
			SpriteComponent->SetVisibility(false);
			break;
		}
	}
}
#endif
