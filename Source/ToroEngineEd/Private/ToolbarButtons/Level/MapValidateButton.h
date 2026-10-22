// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "Styling/AppStyle.h"
#include "ToolbarButtons/ToroToolbarButton.h"
#include "Utilities/ToroValidation.h"
#include "Editor/EditorEngine.h"
#include "Engine/World.h"
#include "Editor.h"

#define LOCTEXT_NAMESPACE "ToroEngine"
class FMapValidateButton final : public FToroToolbarButton
{
public:

	FMapValidateButton()
		: FToroToolbarButton(TEXT("MapValidate"))
	{
		Label = LOCTEXT("MapValidateLabel", "Validate Map");
		Tooltip = LOCTEXT("MapValidateTooltip", "Validates the active editor map's loaded actors and components, including any unsaved changes.");
		MenuHook = TEXT("LevelEditor.LevelEditorToolBar.ModesToolBar");
		SectionName = TEXT("File");
	}

private:

	virtual FSlateIcon GetSlateIcon() override
	{
		return FSlateIcon(FAppStyle::GetAppStyleSetName(), TEXT("Icons.Validate"));
	}

	virtual void Execute(const FToolMenuContext& Context) override
	{
		const UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
		if (IsValid(World) && World->IsEditorWorld())
		{
			ToroEngine::Validation::ValidateWorld(World);
		}
	}
};
#undef LOCTEXT_NAMESPACE