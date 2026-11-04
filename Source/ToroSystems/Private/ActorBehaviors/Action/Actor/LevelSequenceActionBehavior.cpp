// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "ActorBehaviors/Action/Actor/LevelSequenceActionBehavior.h"
#include "Libraries/ToroSequenceLibrary.h"
#include "LevelSequenceActor.h"

void ULevelSequenceActionBehavior::OnTriggered()
{
	if (const ALevelSequenceActor* Actor = Target.Get(); IsValid(Actor))
	{
		UToroSequenceLibrary::PlayLevelSequence(Actor, PlayMode, PlayRate, bResumePlay);
	}
}
