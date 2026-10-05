// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#include "AsyncActions/ToroAsyncActionBase.h"
#include "Utilities/WorldGetter.h"

void UToroAsyncActionBase::SetWorldContext(const UObject* InContext)
{
	if (IsValid(InContext))
	{
		WorldContext = InContext;
	}
}

UWorld* UToroAsyncActionBase::GetWorld() const
{
	return FWorldGetter::Get(GetWorldContext());
}
