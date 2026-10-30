// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "Widgets/GameVersionBlock.h"
#include "GeneralProjectSettings.h"
#include "Misc/App.h"
#if WITH_EDITOR
#include "Widgets/ToroWidgetCategories.h"
#endif

UGameVersionBlock::UGameVersionBlock()
{
	PRAGMA_DISABLE_DEPRECATION_WARNINGS
	Font.Size = 24.0f;
	Font.TypefaceFontName = TEXT("Regular");
	Font.OutlineSettings.OutlineColor = FLinearColor::Black;
	Font.OutlineSettings.OutlineSize = 2;
	PRAGMA_ENABLE_DEPRECATION_WARNINGS

	Justification = ETextJustify::Left;

	bIncludeDebugTitle = true;
}

TSharedRef<SWidget> UGameVersionBlock::RebuildWidget()
{
	FString Result = TEXT("Version: ");
	if (const UGeneralProjectSettings* Settings = GetDefault<UGeneralProjectSettings>())
	{
		Result.Appendf(TEXT("%s-%s"),
			*Settings->ProjectVersion,
			*FString(LexToString(FApp::GetBuildConfiguration())).ToUpper()
		);

		if (bIncludeDebugTitle && !Settings->ProjectDebugTitleInfo.IsEmptyOrWhitespace())
		{
			Result.Appendf(TEXT(" | %s"), *Settings->ProjectDebugTitleInfo.ToString());
		}
	}
	else
	{
		Result.Append(TEXT("Unknown"));
	}

	SetText(FText::FromString(Result));
	return Super::RebuildWidget();
}

#if WITH_EDITOR
const FText UGameVersionBlock::GetPaletteCategory()
{
	return ToroWidgetCategories::Information;
}

bool UGameVersionBlock::CanEditChange(const FProperty* InProperty) const
{
	PRAGMA_DISABLE_DEPRECATION_WARNINGS
	if (InProperty && InProperty->GetName() == GET_MEMBER_NAME_CHECKED(UTextBlock, Text))
	{
		return false;
	}
	PRAGMA_ENABLE_DEPRECATION_WARNINGS

	return Super::CanEditChange(InProperty);
}
#endif
