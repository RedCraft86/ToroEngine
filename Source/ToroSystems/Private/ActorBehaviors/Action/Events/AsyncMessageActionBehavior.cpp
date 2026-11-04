// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "ActorBehaviors/Action/Events/AsyncMessageActionBehavior.h"
#include "AsyncGameplayMessageSystem.h"
#include "AsyncMessageWorldSubsystem.h"
#include "StructUtils/StructView.h"

void UAsyncMessageActionBehavior::OnTriggered()
{
	if (!MessageId.IsValid())
	{
		return;
	}

	const TSharedPtr<FAsyncGameplayMessageSystem> System =
		UAsyncMessageWorldSubsystem::GetSharedMessageSystem<FAsyncGameplayMessageSystem>(GetWorld());
	if (System.IsValid())
	{
		System->QueueMessageForBroadcast(MessageId, FConstStructView(Payload));
	}
}
