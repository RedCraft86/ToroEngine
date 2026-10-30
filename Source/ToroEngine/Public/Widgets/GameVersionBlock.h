// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "CommonTextBlock.h"
#include "GameVersionBlock.generated.h"

/**
 * Displays the project version and uppercase build configuration, optionally followed
 * by the project's debug-title text. The label is generated when the Slate widget is
 * rebuilt and uses "Version: Unknown" if project settings are unavailable.
 */
UCLASS(MinimalAPI, NotBlueprintable, BlueprintType)
class UGameVersionBlock final : public UCommonTextBlock
{
	GENERATED_BODY()

public:

	UGameVersionBlock();

private:

	/**
	 * Appends nonempty project debug-title text after the version label in any build
	 * configuration. Enabled by default; title tokens are displayed literally.
	 */
	UPROPERTY(EditAnywhere, Category = Appearance)
	bool bIncludeDebugTitle;

	virtual TSharedRef<SWidget> RebuildWidget() override;

#if WITH_EDITOR
	virtual const FText GetPaletteCategory() override;
	virtual bool CanEditChange(const FProperty* InProperty) const override;
#endif
};
