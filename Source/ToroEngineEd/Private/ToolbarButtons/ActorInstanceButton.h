// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#pragma once

#include "ToroEngineStyle.h"
#include "Helpers/ToroActorEditorUtils.h"
#include "MiscEditor/ToroToolbarButton.h"

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
		if (const TArray<AActor*> Actors = FToroActorEditorUtils::GetSelectedActors(true); !Actors.IsEmpty())
		{
			if (FMessageDialog::Open(EAppMsgCategory::Info, EAppMsgType::Type::OkCancel,
				LOCTEXT("ActorInstanceDescription",
					"This will collect mesh components from selected actors and group similar ones to use an Instanced Static Mesh."),
				LOCTEXT("ActorInstanceQuestion", "Are you sure you want to instance the selected meshes?")) == EAppReturnType::Cancel)
			{
				return;
			}

			const EAppReturnType::Type HISMRetType = FMessageDialog::Open(
				EAppMsgCategory::Info, EAppMsgType::Type::YesNo,
				LOCTEXT("ActorInstanceHISMDesc",
					"Whether to use Hierarchical Instanced Static Mesh over regular Instanced Static Mesh components."),
				LOCTEXT("ActorInstanceHISMQuestion", "Do you want to use HISMs instead of ISMs?")
			);

			const EAppReturnType::Type DestroySrc = FMessageDialog::Open(
				EAppMsgCategory::Warning, EAppMsgType::Type::YesNo,
				LOCTEXT("DestroyActorsDesc", "Whether to destroy source actors, otherwise they will be kept."),
				LOCTEXT("DestroyActorsQuestion", "Do you want to destroy source actors?")
			);

			FToroActorEditorUtils::InstanceActors(Actors, HISMRetType == EAppReturnType::Yes, DestroySrc == EAppReturnType::Yes);
		}
	}
};
#undef LOCTEXT_NAMESPACE