// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "Dialogues/DialogueActionBehavior.h"
#include "Dialogues/DialogueGraph.h"
#include "Dialogues/DialogueSubsystem.h"

bool UDialogueActionBehavior::CanTrigger() const
{
	return Dialogue && !Dialogue->HasAnyClassFlags(CLASS_Abstract);
}

void UDialogueActionBehavior::OnTriggered()
{
	UDialogueSubsystem::Start(this, Dialogue);
}
