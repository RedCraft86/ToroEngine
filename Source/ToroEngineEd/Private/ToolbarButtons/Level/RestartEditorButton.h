// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "Styling/AppStyle.h"
#include "ToolbarButtons/ToroToolbarButton.h"
#include "UnrealEdMisc.h"

#define LOCTEXT_NAMESPACE "ToroEngine"
class FRestartEditorButton final : public FToroToolbarButton
{
public:

	FRestartEditorButton()
		: FToroToolbarButton(TEXT("RestartEditor"))
	{
		Label = LOCTEXT("RestartEditorLabel", "Restart Editor");
		Tooltip = LOCTEXT("RestartEditorTooltip", "Prompts saving if needed and restarts the Unreal Editor.");
		MenuHook = TEXT("LevelEditor.LevelEditorToolBar.ModesToolBar");
		SectionName = TEXT("File");
	}

private:

	virtual FSlateIcon GetSlateIcon() override
	{
		return FSlateIcon(FCoreStyle::Get().GetStyleSetName(), TEXT("Icons.Refresh"));
	}

	virtual void Execute(const FToolMenuContext& Context) override
	{
		FUnrealEdMisc::Get().RestartEditor(false);
	}
};
#undef LOCTEXT_NAMESPACE