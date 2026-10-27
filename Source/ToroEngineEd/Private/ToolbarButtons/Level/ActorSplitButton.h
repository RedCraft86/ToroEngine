// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "Misc/MessageDialog.h"
#include "SlateStyles/ToroEngineStyle.h"
#include "ToolbarButtons/ToroToolbarButton.h"
#include "Utilities/ToroActorEdUtils.h"

#define LOCTEXT_NAMESPACE "ToroEngine"
class FActorSplitButton final : public FToroToolbarButton
{
public:

	FActorSplitButton()
		: FToroToolbarButton(TEXT("ActorSplit"))
	{
		Label = LOCTEXT("ActorSplitLabel", "Split Actors");
		Tooltip = LOCTEXT("ActorSplitTooltip", "Collects mesh components from the selected actors and turn them into individual actors.");
		MenuHook = TEXT("LevelEditor.LevelEditorToolBar.PlayToolBar");
		SectionName = TEXT("Actor");
	}

private:

	virtual FSlateIcon GetSlateIcon() override
	{
		return FSlateIcon(FToroEngineStyle::StyleName, TEXT("Toolbar.ActorSplit"));
	}

	virtual void Execute(const FToolMenuContext& Context) override
	{
		const TArray<AActor*> Actors = ToroEngine::ActorEdUtils::GetSelectedActors(true);
		if (Actors.IsEmpty())
		{
			return;
		}

		if (FMessageDialog::Open(EAppMsgCategory::Info, EAppMsgType::OkCancel,
			LOCTEXT("ActorSplitDesc", "This will collect mesh components from selected actors and convert them to individual static mesh actors."),
			LOCTEXT("ActorSplitConfirm", "Split selected meshes?")) != EAppReturnType::Ok)
		{
			return;
		}

		const bool bDestroySources = FMessageDialog::Open(EAppMsgCategory::Info, EAppMsgType::YesNo,
			LOCTEXT("DestroySourceActorsDesc", "Whether to destroy source actors or keep them instead."),
			LOCTEXT("DestroySourceActorsMConfirm", "Destroy source actors?")) == EAppReturnType::Yes;

		ToroEngine::ActorEdUtils::SplitActors(Actors, bDestroySources);
	}
};
#undef LOCTEXT_NAMESPACE