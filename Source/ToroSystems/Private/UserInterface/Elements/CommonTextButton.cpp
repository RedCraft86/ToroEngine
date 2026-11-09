// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "UserInterface/Elements/CommonTextButton.h"
#include "Blueprint/WidgetTree.h"
#include "Components/SizeBoxSlot.h"
#include "Components/SizeBox.h"
#include "CommonTextBlock.h"

UCommonTextButton::UCommonTextButton(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	if (const UCommonTextBlock* DefaultTextBlock = GetDefault<UCommonTextBlock>())
	{
		TextFont = DefaultTextBlock->GetFont();
		TextColorAndOpacity = DefaultTextBlock->GetColorAndOpacity();
		TextShadowOffset = DefaultTextBlock->GetShadowOffset();
		TextShadowColorAndOpacity = DefaultTextBlock->GetShadowColorAndOpacity();
		TextStrikeBrush = DefaultTextBlock->GetStrikeBrush();
	}
}

const FText& UCommonTextButton::GetButtonText() const
{
	return ButtonText;
}

void UCommonTextButton::SetButtonText(const FText& InText)
{
	ButtonText = InText;
	ApplyTextProperties();
}

ETextJustify::Type UCommonTextButton::GetTextJustification() const
{
	return TextJustification;
}

void UCommonTextButton::SetTextJustification(const ETextJustify::Type InJustification)
{
	TextJustification = InJustification;
	ApplyTextProperties();
}

bool UCommonTextButton::GetTextAutoWrapText() const
{
	return bTextAutoWrapText;
}

void UCommonTextButton::SetTextAutoWrapText(const bool bInAutoWrapText)
{
	bTextAutoWrapText = bInAutoWrapText;
	ApplyTextProperties();
}

float UCommonTextButton::GetTextWrapTextAt() const
{
	return TextWrapTextAt;
}

void UCommonTextButton::SetTextWrapTextAt(const float InWrapTextAt)
{
	TextWrapTextAt = FMath::Max(0.0f, InWrapTextAt);
	ApplyTextProperties();
}

ETextOverflowPolicy UCommonTextButton::GetTextOverflowPolicy() const
{
	return TextOverflowPolicy;
}

void UCommonTextButton::SetTextOverflowPolicy(const ETextOverflowPolicy InOverflowPolicy)
{
	TextOverflowPolicy = InOverflowPolicy;
	ApplyTextProperties();
}

const FSlateFontInfo& UCommonTextButton::GetTextFont() const
{
	return TextFont;
}

void UCommonTextButton::SetTextFont(const FSlateFontInfo& InFont)
{
	TextFont = InFont;
	ApplyTextProperties();
}

FSlateColor UCommonTextButton::GetTextColorAndOpacity() const
{
	return TextColorAndOpacity;
}

void UCommonTextButton::SetTextColorAndOpacity(const FSlateColor InColorAndOpacity)
{
	TextColorAndOpacity = InColorAndOpacity;
	ApplyTextProperties();
}

FVector2D UCommonTextButton::GetTextShadowOffset() const
{
	return TextShadowOffset;
}

void UCommonTextButton::SetTextShadowOffset(const FVector2D InShadowOffset)
{
	TextShadowOffset = InShadowOffset;
	ApplyTextProperties();
}

FLinearColor UCommonTextButton::GetTextShadowColorAndOpacity() const
{
	return TextShadowColorAndOpacity;
}

void UCommonTextButton::SetTextShadowColorAndOpacity(const FLinearColor InShadowColorAndOpacity)
{
	TextShadowColorAndOpacity = InShadowColorAndOpacity;
	ApplyTextProperties();
}

const FMargin& UCommonTextButton::GetTextMargin() const
{
	return TextMargin;
}

void UCommonTextButton::SetTextMargin(const FMargin& InMargin)
{
	TextMargin = InMargin;
	ApplyTextProperties();
}

const FSlateBrush& UCommonTextButton::GetTextStrikeBrush() const
{
	return TextStrikeBrush;
}

void UCommonTextButton::SetTextStrikeBrush(const FSlateBrush& InStrikeBrush)
{
	TextStrikeBrush = InStrikeBrush;
	ApplyTextProperties();
}

float UCommonTextButton::GetTextLineHeightPercentage() const
{
	return TextLineHeightPercentage;
}

void UCommonTextButton::SetTextLineHeightPercentage(const float InLineHeightPercentage)
{
	TextLineHeightPercentage = InLineHeightPercentage;
	ApplyTextProperties();
}

bool UCommonTextButton::GetTextApplyLineHeightToBottomLine() const
{
	return bTextApplyLineHeightToBottomLine;
}

void UCommonTextButton::SetTextApplyLineHeightToBottomLine(const bool bInApplyLineHeightToBottomLine)
{
	bTextApplyLineHeightToBottomLine = bInApplyLineHeightToBottomLine;
	ApplyTextProperties();
}

void UCommonTextButton::ApplyTextProperties() const
{
	if (!TextBlock)
	{
		return;
	}

	const TSubclassOf<UCommonTextStyle> TextStyle = GetCurrentTextStyleClass();
	TextBlock->SetStyle(TextStyle);
	if (!TextStyle)
	{
		TextBlock->SetFont(TextFont);
		TextBlock->SetColorAndOpacity(TextColorAndOpacity);
		TextBlock->SetShadowOffset(TextShadowOffset);
		TextBlock->SetShadowColorAndOpacity(TextShadowColorAndOpacity);
		TextBlock->SetMargin(TextMargin);
		TextBlock->SetStrikeBrush(TextStrikeBrush);
		TextBlock->SetLineHeightPercentage(TextLineHeightPercentage);
		TextBlock->SetApplyLineHeightToBottomLine(bTextApplyLineHeightToBottomLine);
	}

	TextBlock->SetText(ButtonText);
	TextBlock->SetJustification(TextJustification);
	TextBlock->SetAutoWrapText(bTextAutoWrapText);
	TextBlock->SetWrapTextAt(TextWrapTextAt);
	TextBlock->SetTextOverflowPolicy(TextOverflowPolicy);
}

void UCommonTextButton::SynchronizeProperties()
{
	Super::SynchronizeProperties();
	ApplyTextProperties();
}

void UCommonTextButton::NativeOnCurrentTextStyleChanged()
{
	Super::NativeOnCurrentTextStyleChanged();
	ApplyTextProperties();
}

UCommonButtonInternalBase* UCommonTextButton::ConstructInternalButton()
{
	USizeBox* Content = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("TextContainer"));
	Content->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	WidgetTree->RootWidget = Content;

	TextBlock = WidgetTree->ConstructWidget<UCommonTextBlock>(UCommonTextBlock::StaticClass(), TEXT("TextBlock"));
	TextBlock->SetVisibility(ESlateVisibility::HitTestInvisible);
	ApplyTextProperties();

	USizeBoxSlot* TextSlot = CastChecked<USizeBoxSlot>(Content->AddChild(TextBlock));
	TextSlot->SetHorizontalAlignment(HAlign_Fill);
	TextSlot->SetVerticalAlignment(VAlign_Center);

	return Super::ConstructInternalButton();
}

#if WITH_EDITOR
void UCommonTextButton::OnCreationFromPalette()
{
	Super::OnCreationFromPalette();
	SetButtonText(NSLOCTEXT("UMG", "TextBlockDefaultValue", "Text Block"));
}
#endif
