// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "UserInterface/Elements/CommonButton.h"
#include "Blueprint/WidgetTree.h"
#include "Components/NamedSlot.h"

const FName UCommonButton::ContentSlotName(TEXT("Content"));

UWidget* UCommonButton::GetContent() const
{
	return GetContentForSlot(ContentSlotName);
}

void UCommonButton::SetContent(UWidget* InContent)
{
	SetContentForSlot(ContentSlotName, InContent);
}

void UCommonButton::GetSlotNames(TArray<FName>& SlotNames) const
{
	Super::GetSlotNames(SlotNames);
	SlotNames.AddUnique(ContentSlotName);
}

UCommonButtonInternalBase* UCommonButton::ConstructInternalButton()
{
	Content = WidgetTree->ConstructWidget<UNamedSlot>(UNamedSlot::StaticClass(), ContentSlotName);
	Content->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	WidgetTree->RootWidget = Content;

	if (UWidget* BoundContent = GetContent())
	{
		Content->SetContent(BoundContent);
	}

	return Super::ConstructInternalButton();
}
