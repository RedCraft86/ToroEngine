// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "Layout/Margin.h"
#include "CommonButtonBase.h"
#include "Styling/SlateBrush.h"
#include "Styling/SlateColor.h"
#include "Styling/SlateTypes.h"
#include "Fonts/SlateFontInfo.h"
#include "Framework/Text/TextLayout.h"
#include "CommonTextButton.generated.h"

class UCommonTextBlock;

/**
 * CommonUI button with a built-in, vertically centered text block.
 * Owns its content layout and uses the current button text style when available,
 * otherwise falling back to its editable text appearance.
 */
UCLASS(NotBlueprintable, BlueprintType, PrioritizeCategories = (Style))
class TOROSYSTEMS_API UCommonTextButton final : public UCommonButtonBase
{
	GENERATED_BODY()

public:

	UCommonTextButton(const FObjectInitializer& ObjectInitializer);

	/** Returns the configured button text. */
	const FText& GetButtonText() const;

	/** Updates the displayed text, including when called before initialization. */
	UFUNCTION(BlueprintCallable, Category = Text)
	void SetButtonText(const FText& InText);

	/** Returns the configured horizontal text alignment. */
	ETextJustify::Type GetTextJustification() const;

	/** Updates horizontal alignment of the text. */
	UFUNCTION(BlueprintCallable, Category = Text)
	void SetTextJustification(ETextJustify::Type InJustification);

	/** Whether the text wraps to the available layout width. */
	bool GetTextAutoWrapText() const;

	/** Enables or disables wrapping to the available layout width. */
	UFUNCTION(BlueprintCallable, Category = Text)
	void SetTextAutoWrapText(bool bInAutoWrapText);

	/** Returns the explicit wrapping width, or zero when unset. */
	float GetTextWrapTextAt() const;

	/** Sets an explicit wrapping width; zero disables the explicit width. */
	UFUNCTION(BlueprintCallable, Category = Text)
	void SetTextWrapTextAt(float InWrapTextAt);

	/** Returns the configured text overflow behavior. */
	ETextOverflowPolicy GetTextOverflowPolicy() const;

	/** Updates the text's clipping or ellipsis behavior. */
	UFUNCTION(BlueprintCallable, Category = Text)
	void SetTextOverflowPolicy(ETextOverflowPolicy InOverflowPolicy);

	/** Returns the fallback font configuration, even when a text style is active. */
	const FSlateFontInfo& GetTextFont() const;

	/** Updates the fallback font, including its outline and font material. */
	UFUNCTION(BlueprintCallable, Category = Text)
	void SetTextFont(const FSlateFontInfo& InFont);

	/** Returns the fallback text tint and opacity, even when a text style is active. */
	FSlateColor GetTextColorAndOpacity() const;

	/** Updates the fallback text tint and opacity. */
	UFUNCTION(BlueprintCallable, Category = Text)
	void SetTextColorAndOpacity(FSlateColor InColorAndOpacity);

	/** Returns the fallback shadow offset, even when a text style is active. */
	FVector2D GetTextShadowOffset() const;

	/** Updates the fallback shadow offset. */
	UFUNCTION(BlueprintCallable, Category = Text)
	void SetTextShadowOffset(FVector2D InShadowOffset);

	/** Returns the fallback shadow tint and opacity, even when a text style is active. */
	FLinearColor GetTextShadowColorAndOpacity() const;

	/** Updates the fallback shadow tint and opacity. */
	UFUNCTION(BlueprintCallable, Category = Text)
	void SetTextShadowColorAndOpacity(FLinearColor InShadowColorAndOpacity);

	/** Returns the fallback text margin, even when a text style is active. */
	const FMargin& GetTextMargin() const;

	/** Updates the fallback spacing around the text. */
	UFUNCTION(BlueprintCallable, Category = Text)
	void SetTextMargin(const FMargin& InMargin);

	/** Returns the fallback strike-through brush, even when a text style is active. */
	const FSlateBrush& GetTextStrikeBrush() const;

	/** Updates the fallback brush used to draw a strike through the text. */
	UFUNCTION(BlueprintCallable, Category = Text)
	void SetTextStrikeBrush(const FSlateBrush& InStrikeBrush);

	/** Returns the fallback line-height multiplier, even when a text style is active. */
	float GetTextLineHeightPercentage() const;

	/** Updates the fallback multiplier applied to each text line's height. */
	UFUNCTION(BlueprintCallable, Category = Text)
	void SetTextLineHeightPercentage(float InLineHeightPercentage);

	/** Whether fallback line-height scaling adds space below the final line. */
	bool GetTextApplyLineHeightToBottomLine() const;

	/** Sets whether fallback line-height scaling also applies below the final line. */
	UFUNCTION(BlueprintCallable, Category = Text)
	void SetTextApplyLineHeightToBottomLine(bool bInApplyLineHeightToBottomLine);

protected:

	/** Localized text displayed inside the button. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Text", meta = (ExposeOnSpawn = true))
	FText ButtonText;

	/** Horizontal text alignment within the available content width. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Text|Layout")
	TEnumAsByte<ETextJustify::Type> TextJustification = ETextJustify::Center;

	/** Whether the text wraps to the available layout width. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Text|Wrapping")
	bool bTextAutoWrapText = false;

	/** Explicit wrapping width in Slate units; zero disables explicit wrapping. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Text|Wrapping", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float TextWrapTextAt = 0.0f;

	/** How text is presented when it exceeds the allocated width. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Text")
	ETextOverflowPolicy TextOverflowPolicy = ETextOverflowPolicy::Clip;

	/** Fallback font, outline, and material when no current button text style resolves. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Text|Appearance")
	FSlateFontInfo TextFont;

	/** Fallback text tint and opacity when no current button text style resolves. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Text|Appearance")
	FSlateColor TextColorAndOpacity;

	/** Fallback shadow offset when no current button text style resolves. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Text|Shadow")
	FVector2D TextShadowOffset;

	/** Fallback shadow tint and opacity when no current button text style resolves. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Text|Shadow")
	FLinearColor TextShadowColorAndOpacity;

	/** Fallback spacing around the text when no current button text style resolves. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Text|Layout")
	FMargin TextMargin = FMargin(0.0f);

	/** Fallback strike-through brush when no current button text style resolves. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Text|Appearance")
	FSlateBrush TextStrikeBrush;

	/** Fallback line-height multiplier when no current button text style resolves. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Text|Layout")
	float TextLineHeightPercentage = 1.0f;

	/** Whether fallback line-height scaling adds space below the final line. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Text|Layout")
	bool bTextApplyLineHeightToBottomLine = true;

	UPROPERTY(Transient)
	TObjectPtr<UCommonTextBlock> TextBlock;

	void ApplyTextProperties() const;

	virtual void SynchronizeProperties() override;
	virtual void NativeOnCurrentTextStyleChanged() override;
	virtual UCommonButtonInternalBase* ConstructInternalButton() override;

#if WITH_EDITOR
	virtual void OnCreationFromPalette() override;
#endif
};
