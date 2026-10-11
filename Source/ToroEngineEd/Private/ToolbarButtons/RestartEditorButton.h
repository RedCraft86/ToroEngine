// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#pragma once

#include "MiscEditor/ToroToolbarButton.h"

#define LOCTEXT_NAMESPACE "ToroEngineEd"
class FRestartEditorButton final : public FToroToolbarButton
{
public:

	FRestartEditorButton()
		: FToroToolbarButton(
			TEXT("RestartEditor"),
			LOCTEXT("RestartEditorLabel", "Restart Editor"),
			LOCTEXT("RestartEditorTooltip", "Prompts saving if needed and restarts the Unreal Editor.")
		)
	{
		MenuHook = TEXT("LevelEditor.LevelEditorToolBar.ModesToolBar");
		SectionName = TEXT("File");
	}

private:

	virtual FSlateIcon GetSlateIcon() override
	{
		return FSlateIcon(FCoreStyle::Get().GetStyleSetName(), TEXT("Icons.Refresh"));
	}

	virtual void Execute() override
	{
		FUnrealEdMisc::Get().RestartEditor(false);
	}
};
#undef LOCTEXT_NAMESPACE