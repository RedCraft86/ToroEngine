// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "Utilities/ToroUtilities.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

UWorld* ToroEngine::World::Get(const UObject* Context, bool bTryFindPossible)
{
	checkf(IsInGameThread(), TEXT("ToroEngine::World::Get(...) should only be called from the GameThread."));

	UWorld* World = nullptr;
	if (IsValid(Context))
	{
		World = Context->GetWorld();
	}

	if (!IsValid(World) && bTryFindPossible)
	{
		World = GEngine ? GEngine->GetCurrentPlayWorld() : nullptr;
		if (!IsValid(World))
		{
			World = GWorld.GetReference();
		}
	}

	return IsValid(World) ? World : nullptr;
}

FLatentActionInfo ToroEngine::Latent::MakeInfo(UObject* Target, const FString& FuncName)
{
	checkf(IsInGameThread(), TEXT("ToroEngine::Latent::MakeInfo(...) should only be called from the GameThread."));

	static int32 LatentId = 1000;
	if (LatentId == TNumericLimits<int32>::Max())
	{
		LatentId = 1000;
	}

	return FLatentActionInfo(0, LatentId++, *FuncName, Target);
}
