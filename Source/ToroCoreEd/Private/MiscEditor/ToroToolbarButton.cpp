// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#include "MiscEditor/ToroToolbarButton.h"

void FToroToolbarButton::Register()
{
	if (!bRegistered)
	{
		FToolMenuOwnerScoped OwnerScoped(this);
		if (UToolMenu* Menu = UToolMenus::Get()->ExtendMenu(MenuHook))
		{
			FToolMenuSection& Section = Menu->FindOrAddSection(SectionName);
			Section.AddEntry(FToolMenuEntry::InitToolBarButton(
				Name,
				FExecuteAction::CreateRaw(this, &FToroToolbarButton::Execute),
				Label,
				Tooltip,
				GetSlateIcon()
			));
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
