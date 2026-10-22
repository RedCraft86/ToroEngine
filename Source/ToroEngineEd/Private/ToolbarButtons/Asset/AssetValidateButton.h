// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "Styling/AppStyle.h"
#include "ToolbarButtons/ToroToolbarButton.h"
#include "Toolkits/AssetEditorToolkitMenuContext.h"
#include "Utilities/ToroValidation.h"
#include "DataAssets/ToroDataAsset.h"

#define LOCTEXT_NAMESPACE "ToroEngine"
class FAssetValidateButton final : public FToroToolbarButton
{
public:

	FAssetValidateButton()
		: FToroToolbarButton(TEXT("AssetValidate"))
	{
		Label = LOCTEXT("AssetValidateLabel", "Validate");
		Tooltip = LOCTEXT("AssetValidateTooltip", "Refreshes and validates the data in the current data asset.");
		MenuHook = TEXT("AssetEditor.DefaultToolBar");
		SectionName = TEXT("Asset");
		bConditional = true;
	}

private:

	virtual FSlateIcon GetSlateIcon() override
	{
		return FSlateIcon(FAppStyle::GetAppStyleSetName(), TEXT("Icons.Validate"));
	}

	virtual bool ShouldShowEntry(const FToolMenuContext& Context) const override
	{
		if (const UAssetEditorToolkitMenuContext* Editor = Context.FindContext<UAssetEditorToolkitMenuContext>())
		{
			return Editor->GetEditingObjects().ContainsByPredicate([](const UObject* Object) -> bool
			{
				return IsValid(Object) && Object->IsA<UToroDataAsset>();
			});
		}

		return false;
	}

	virtual void Execute(const FToolMenuContext& Context) override
	{
		if (const UAssetEditorToolkitMenuContext* Editor = Context.FindContext<UAssetEditorToolkitMenuContext>())
		{
			for (UObject* Object : Editor->GetEditingObjects())
			{
				if (UToroDataAsset* Asset = Cast<UToroDataAsset>(Object))
				{
					Asset->RefreshData();
					ToroEngine::Validation::ValidateAsset(Asset);
				}
			}
		}
	}
};
#undef LOCTEXT_NAMESPACE