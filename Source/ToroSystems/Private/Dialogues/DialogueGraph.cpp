// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "Dialogues/DialogueGraph.h"
#include "Dialogues/DialogueNodes.h"

namespace DialogueTags
{
	UE_DEFINE_GAMEPLAY_TAG(Base, "Dialogue");
}

UDialogueRootNode::UDialogueRootNode()
{
#if WITH_EDITORONLY_DATA
	bHideFromDropDownIfRulesFail = true;
	bHideFromContextMenuIfRulesFail = true;
	ConnectionRules.AllowedInStateMachines.Emplace_GetRef(StaticClass()).bIncludeChildren = false;
	NodePlacementRules.AllowedStates.Emplace_GetRef(UDialogueNodeBase::StaticClass()).bIncludeChildren = true;
#endif
}

UDialogueGraph::UDialogueGraph()
{
	SetRootStateMachineNodeClass(UDialogueRootNode::StaticClass());
}
