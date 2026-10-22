// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "Styling/AppStyle.h"
#include "ToolbarButtons/ToroToolbarButton.h"
#include "Toolkits/AssetEditorToolkitMenuContext.h"
#include "DataAssets/ToroDataAsset.h"

#define LOCTEXT_NAMESPACE "ToroEngine"
class FAssetRefreshButton final : public FToroToolbarButton
{
public:

	FAssetRefreshButton()
		: FToroToolbarButton(TEXT("AssetRefresh"))
	{
		Label = LOCTEXT("AssetRefreshLabel", "Refresh");
		Tooltip = LOCTEXT("AssetRefreshTooltip", "Refreshes the data in the current data asset without validating.");
		MenuHook = TEXT("AssetEditor.DefaultToolBar");
		SectionName = TEXT("Asset");
		bConditional = true;
	}

private:

	virtual FSlateIcon GetSlateIcon() override
	{
		return FSlateIcon(TEXT("DefaultRevisionControlStyle"), TEXT("RevisionControl.Actions.Refresh"));
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
				}
			}
		}
	}
};
#undef LOCTEXT_NAMESPACE