// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "Objectives/ObjectiveGraph.h"
#include "Objectives/ObjectiveNodes.h"

UObjectiveRootNode::UObjectiveRootNode()
{
#if WITH_EDITORONLY_DATA
	bHideFromDropDownIfRulesFail = true;
	bHideFromContextMenuIfRulesFail = true;
	ConnectionRules.AllowedInStateMachines.Emplace_GetRef(StaticClass()).bIncludeChildren = false;
	NodePlacementRules.AllowedStates.Emplace_GetRef(UObjectiveNode::StaticClass()).bIncludeChildren = true;
#endif
}

UObjectiveGraph::UObjectiveGraph()
{
	SetRootStateMachineNodeClass(UObjectiveRootNode::StaticClass());
}
