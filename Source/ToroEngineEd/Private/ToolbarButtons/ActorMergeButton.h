// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#pragma once

#include "ToroEngineEd.h"
#include "ToroEngineStyle.h"
#include "MiscEditor/ToroToolbarButton.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Framework/Notifications/NotificationManager.h"

#define LOCTEXT_NAMESPACE "ToroEngineEd"
class FActorMergeButton final : public FToroToolbarButton
{
public:

	FActorMergeButton()
		: FToroToolbarButton(
			TEXT("ActorMerge"),
			LOCTEXT("ActorMergeLabel", "Actor Merge"),
			LOCTEXT("ActorMergeTooltip", "Opens the Actor Merge screen for the selected actors.")
		)
	{
		MenuHook = TEXT("LevelEditor.LevelEditorToolBar.PlayToolBar");
		SectionName = TEXT("Actor");
	}

private:

	virtual FSlateIcon GetSlateIcon() override
	{
		return FSlateIcon(FToroEngineStyle::StyleName, TEXT("Toolbar.ActorMerge"));
	}

	virtual void Execute() override
	{
		if (!FToroEngineEdModule::ActorSubsystem.Get() || FToroEngineEdModule::ActorSubsystem->GetSelectedLevelActors().IsEmpty())
		{
			static const FNotificationInfo Info(LOCTEXT("NoActorsSelectedNotification", "No Actors Selected!"));
			FSlateNotificationManager::Get().AddNotification(Info);
			return;
		}

		FGlobalTabmanager::Get()->TryInvokeTab(FTabId(TEXT("MergeActors")));
	}
};
#undef LOCTEXT_NAMESPACE