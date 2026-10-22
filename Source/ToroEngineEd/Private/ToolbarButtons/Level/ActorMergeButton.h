// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "SlateStyles/ToroEngineStyle.h"
#include "ToolbarButtons/ToroToolbarButton.h"
#include "Utilities/ToroActorEdUtils.h"

#define LOCTEXT_NAMESPACE "ToroEngine"
class FActorMergeButton final : public FToroToolbarButton
{
public:

	FActorMergeButton()
		: FToroToolbarButton(TEXT("ActorMerge"))
	{
		Label = LOCTEXT("ActorMergeLabel", "Merge Actors");
		Tooltip = LOCTEXT("ActorMergeTooltip", "Opens the Merge Actors tab to merge selected actors into one.");
		MenuHook = TEXT("LevelEditor.LevelEditorToolBar.PlayToolBar");
		SectionName = TEXT("Actor");
	}

private:

	virtual FSlateIcon GetSlateIcon() override
	{
		return FSlateIcon(FToroEngineStyle::StyleName, TEXT("Toolbar.ActorMerge"));
	}

	virtual void Execute(const FToolMenuContext& Context) override
	{
		static FTabId MergeActorsTab(TEXT("MergeActors"));
		if (!ToroEngine::ActorEdUtils::GetSelectedActors(true).IsEmpty())
		{
			FGlobalTabmanager::Get()->TryInvokeTab(MergeActorsTab);
		}
	}
};
#undef LOCTEXT_NAMESPACE