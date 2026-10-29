// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "Misc/MessageDialog.h"
#include "SlateStyles/ToroEngineStyle.h"
#include "ToolbarButtons/ToroToolbarButton.h"
#include "Utilities/ToroActorEdUtils.h"

#define LOCTEXT_NAMESPACE "ToroEngine"
class FActorInstanceButton final : public FToroToolbarButton
{
public:

	FActorInstanceButton()
		: FToroToolbarButton(TEXT("ActorInstance"))
	{
		Label = LOCTEXT("ActorInstanceLabel", "Instance Actors");
		Tooltip = LOCTEXT("ActorInstanceTooltip", "Groups similar mesh components from selected actors to use Instanced Static Mesh actors.");
		MenuHook = TEXT("LevelEditor.LevelEditorToolBar.PlayToolBar");
		SectionName = TEXT("Actor");
	}

private:

	virtual FSlateIcon GetSlateIcon() override
	{
		return FSlateIcon(FToroEngineStyle::StyleName, TEXT("Toolbar.ActorInstance"));
	}

	virtual void Execute(const FToolMenuContext& Context) override
	{
		const TArray<AActor*> Actors = ToroEngineEd::ActorUtils::GetSelectedActors(true);
		if (Actors.IsEmpty())
		{
			return;
		}

		if (FMessageDialog::Open(EAppMsgCategory::Info, EAppMsgType::OkCancel,
			LOCTEXT("ActorInstanceDesc", "This will collect mesh components from selected actors and group similar ones to Instanced Static Meshes."),
			LOCTEXT("ActorInstanceConfirm", "Instance selected meshes?")) != EAppReturnType::Ok)
		{
			return;
		}

		const bool bUseHISM = FMessageDialog::Open(EAppMsgCategory::Info, EAppMsgType::YesNo,
			LOCTEXT("ActorInstanceHISMDesc", "Whether to use Hierarchical Instanced Static Mesh over regular Instanced Static Meshes."),
			LOCTEXT("ActorInstanceHISMConfirm", "Use HISMs instead of ISMs?")) == EAppReturnType::Yes;

		const bool bDestroySources = FMessageDialog::Open(EAppMsgCategory::Info, EAppMsgType::YesNo,
			LOCTEXT("DestroySourceActorsDesc", "Whether to destroy source actors or keep them instead."),
			LOCTEXT("DestroySourceActorsMConfirm", "Destroy source actors?")) == EAppReturnType::Yes;

		ToroEngineEd::ActorUtils::InstanceActors(Actors, bUseHISM, bDestroySources);
	}
};
#undef LOCTEXT_NAMESPACE