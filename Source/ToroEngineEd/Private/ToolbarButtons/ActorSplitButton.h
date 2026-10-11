// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#pragma once

#include "Selection.h"
#include "ToroEngineStyle.h"
#include "MiscEditor/ToroToolbarButton.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Framework/Notifications/NotificationManager.h"

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
		if (!GEditor->GetSelectedActors() || GEditor->GetSelectedActors()->Num() == 0)
		{
			static const FNotificationInfo Info(LOCTEXT("NoActorsSelectedNotification", "No Actors Selected!"));
			FSlateNotificationManager::Get().AddNotification(Info);
			return;
		}

		// TODO impl feature
	}
};
#undef LOCTEXT_NAMESPACE