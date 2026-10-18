// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#pragma once

#include "MiscEditor/ToroToolbarButton.h"
#include "Utilities/ToroValidation.h"
#include "Styling/AppStyle.h"
#include "Engine/World.h"
#include "Editor.h"

#define LOCTEXT_NAMESPACE "ToroEngineEd"
class FValidateMapButton final : public FToroToolbarButton
{
public:

	FValidateMapButton()
		: FToroToolbarButton(
			TEXT("ValidateMap"),
			LOCTEXT("ValidateMapLabel", "Validate Map"),
			LOCTEXT("ValidateMapTooltip", "Validates the active editor map's loaded actors and components, including unsaved changes.")
		)
	{
		MenuHook = TEXT("LevelEditor.LevelEditorToolBar.ModesToolBar");
		SectionName = TEXT("File");
	}

private:

	virtual FSlateIcon GetSlateIcon() override
	{
		return FSlateIcon(FAppStyle::GetAppStyleSetName(), TEXT("Icons.Validate"));
	}

	virtual void Execute() override
	{
		const UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
		if (IsValid(World) && World->WorldType == EWorldType::Editor)
		{
			ToroEngine::Validation::ValidateWorld(World);
		}
	}
};
#undef LOCTEXT_NAMESPACE