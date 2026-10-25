// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "AsyncActions/ToroAsyncActionBase.h"
#include "Utilities/ToroUtilities.h"

UToroAsyncActionBase::UToroAsyncActionBase(): bAllowAnyWorld(false)
{
}

void UToroAsyncActionBase::SetWorldContext(const UObject* InContext)
{
	if (IsValid(InContext))
	{
		WorldContext = InContext;
	}
}

UWorld* UToroAsyncActionBase::GetWorld() const
{
	return IsTemplate() ? nullptr : ToroEngine::World::Get(GetWorldContext(), bAllowAnyWorld);
}
