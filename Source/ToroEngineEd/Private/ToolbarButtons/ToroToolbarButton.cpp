// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "ToolbarButtons/ToroToolbarButton.h"
#include "Styling/CoreStyle.h"
#include "ToolMenu.h"
#include "ToolMenuEntry.h"
#include "ToolMenuSection.h"

void FToroToolbarButton::Register()
{
	if (!bRegistered && !Name.IsNone() && !MenuHook.IsNone() && !SectionName.IsNone())
	{
		if (Label.IsEmptyOrWhitespace())
		{
			Label = FText::FromString(FName::NameToDisplayString(Name.ToString(), false));
		}

		FToolMenuOwnerScoped OwnerScoped(this);
		if (UToolMenu* Toolbar = UToolMenus::Get()->ExtendMenu(MenuHook))
		{
			FToolMenuSection& Section = Toolbar->FindOrAddSection(SectionName);
			if (bConditional)
			{
				Section.AddDynamicEntry(Name, FNewToolMenuSectionDelegate::CreateSP(
					AsShared(), &FToroToolbarButton::BuildButtonEntry
				));
			}
			else
			{
				BuildButtonEntry(Section);
			}

			bRegistered = true;
		}
	}
}

void FToroToolbarButton::Unregister()
{
	UToolMenus::UnRegisterStartupCallback(this);
	if (bRegistered)
	{
		bRegistered = false;
		UToolMenus::UnregisterOwner(this);
	}
}

void FToroToolbarButton::BuildButtonEntry(FToolMenuSection& Section)
{
	if (!bConditional || ShouldShowEntry(Section.Context))
	{
		FToolMenuOwnerScoped OwnerScoped(this);
		Section.AddEntry(FToolMenuEntry::InitToolBarButton(Name,
			FToolUIAction(FToolMenuExecuteAction::CreateSP(
				AsShared(), &FToroToolbarButton::Execute
			)), Label, Tooltip, GetSlateIcon()
		));
	}
}

bool FToroToolbarButton::ShouldShowEntry(const FToolMenuContext& Context) const
{
	return true;
}

FSlateIcon FToroToolbarButton::GetSlateIcon()
{
	return FSlateIcon(FCoreStyle::Get().GetStyleSetName(), TEXT("Launcher.Instance_Unknown"));
}
