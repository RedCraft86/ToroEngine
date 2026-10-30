// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "Components/WireShapeComponent.h"
#if WITH_EDITOR
#include "Containers/Ticker.h"
#include "Misc/App.h"
#endif

UWireShapeComponent::UWireShapeComponent()
{
#if WITH_EDITOR
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	PrimaryComponentTick.TickGroup = TG_DuringPhysics;
	PrimaryComponentTick.TickInterval = 0.1f;
	bTickInEditor = true;
	bAutoActivate = false;

	SetIsVisualizationComponent(true);
#else
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
#endif
}

#if WITH_EDITOR
void UWireShapeComponent::BeginPlay()
{
	Super::BeginPlay();
	SetComponentTickEnabled(false);
	ExecuteOnGameThread(TEXT("DestroyWireShapeComponent"), [WeakThis = TWeakObjectPtr(this)]()
	{
		if (WeakThis.IsValid())
		{
			WeakThis->DestroyComponent();
		}
	});
}

void UWireShapeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* TickFunc)
{
	Super::TickComponent(DeltaTime, TickType, TickFunc);
	if (!WireNavPaths.IsEmpty() && !FApp::IsGame() && IsSelectedInEditor())
	{
		for (TPair<FName, FWireNavPathData>& NavPath : WireNavPaths)
		{
			NavPath.Value.GeneratePathPoints(GetOwner());
			NavPath.Value.FindNearestToCamera();
		}
	}
}
#endif
