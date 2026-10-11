// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#pragma once

#include "Selection.h"
#include "ToroEngineStyle.h"
#include "MiscEditor/ToroToolbarButton.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Framework/Notifications/NotificationManager.h"

#define LOCTEXT_NAMESPACE "ToroEngineEd"
class FActorLayoutButton final : public FToroToolbarButton
{
public:

	FActorLayoutButton()
		: FToroToolbarButton(
			TEXT("ActorLayout"),
			LOCTEXT("ActorLayoutLabel", "Actor Layout"),
			LOCTEXT("ActorLayoutTooltip", "Layouts the selected actors using the provided settings.")
		)
	{
		MenuHook = TEXT("LevelEditor.LevelEditorToolBar.PlayToolBar");
		SectionName = TEXT("Actor");
	}

private:

	virtual FSlateIcon GetSlateIcon() override
	{
		return FSlateIcon(FToroEngineStyle::StyleName, TEXT("Toolbar.ActorLayout"));
	}

	virtual void Execute() override
	{
		if (!GEditor->GetSelectedActors() || GEditor->GetSelectedActors()->Num() == 0)
		{
			static const FNotificationInfo Info(LOCTEXT("NoActorsSelectedNotification", "No Actors Selected!"));
			FSlateNotificationManager::Get().AddNotification(Info);
			return;
		}

		int32 MaxColumns = 10;
		float YawOffset = 0.0f;
		FVector2D XYOffset = FVector2D(100.0f);

		// TODO impl feature
	}
};
#undef LOCTEXT_NAMESPACE