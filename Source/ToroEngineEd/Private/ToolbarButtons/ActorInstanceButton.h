// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#pragma once

#include "Selection.h"
#include "ToroEngineStyle.h"
#include "MiscEditor/ToroToolbarButton.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Framework/Notifications/NotificationManager.h"

#define LOCTEXT_NAMESPACE "ToroEngineEd"
class FActorInstanceButton final : public FToroToolbarButton
{
public:

	FActorInstanceButton()
		: FToroToolbarButton(
			TEXT("ActorInstance"),
			LOCTEXT("ActorInstanceLabel", "Actor Instance"),
			LOCTEXT("ActorInstanceTooltip", "Collects mesh components from selected actors and groups similar ones to use an Instanced Static Mesh.")
		)
	{
		MenuHook = TEXT("LevelEditor.LevelEditorToolBar.PlayToolBar");
		SectionName = TEXT("Actor");
	}

private:

	virtual FSlateIcon GetSlateIcon() override
	{
		return FSlateIcon(FToroEngineStyle::StyleName, TEXT("Toolbar.ActorInstance"));
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