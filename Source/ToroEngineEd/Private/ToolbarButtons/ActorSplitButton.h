// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#pragma once

#include "ToroEngineStyle.h"
#include "Helpers/ToroActorEditorUtils.h"
#include "MiscEditor/ToroToolbarButton.h"

#define LOCTEXT_NAMESPACE "ToroEngineEd"
class FActorSplitButton final : public FToroToolbarButton
{
public:

	FActorSplitButton()
		: FToroToolbarButton(
			TEXT("ActorSplit"),
			LOCTEXT("ActorSplitLabel", "Actor Split"),
			LOCTEXT("ActorSplitTooltip", "Collects mesh components from selected actors and spawn individual actors from them.")
		)
	{
		MenuHook = TEXT("LevelEditor.LevelEditorToolBar.PlayToolBar");
		SectionName = TEXT("Actor");
	}

private:

	virtual FSlateIcon GetSlateIcon() override
	{
		return FSlateIcon(FToroEngineStyle::StyleName, TEXT("Toolbar.ActorSplit"));
	}

	virtual void Execute() override
	{
		if (const TArray<AActor*> Actors = FToroActorEditorUtils::GetSelectedActors(true); !Actors.IsEmpty())
		{
			if (FMessageDialog::Open(EAppMsgCategory::Info, EAppMsgType::Type::OkCancel,
				LOCTEXT("ActorSplitDescription",
					"This will collect mesh components from the selected actors and spawn individual actors from them."),
				LOCTEXT("ActorSplitQuestion", "Are you sure you want to split selected meshes?")) == EAppReturnType::Cancel)
			{
				return;
			}

			const EAppReturnType::Type DestroySrc = FMessageDialog::Open(
				EAppMsgCategory::Warning, EAppMsgType::Type::YesNo,
				LOCTEXT("DestroyActorsDesc", "Whether to destroy source actors, otherwise they will be kept."),
				LOCTEXT("DestroyActorsQuestion", "Do you want to destroy source actors?")
			);

			FToroActorEditorUtils::SplitActors(Actors, DestroySrc == EAppReturnType::Yes);
		}
	}
};
#undef LOCTEXT_NAMESPACE