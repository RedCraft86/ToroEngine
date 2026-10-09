// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#pragma once

#include "ToroCoreEd.h"
#include "Styling/SlateStyle.h"
#include "Styling/SlateStyleRegistry.h"

/**
 * Slate style base that owns its brushes and tracks registered styles for module cleanup.
 */
class TOROCOREED_API FToroSlateStyle : protected FSlateStyleSet, public TSharedFromThis<FToroSlateStyle>
{
	static inline TMap<FName, TSharedPtr<FToroSlateStyle>> StyleSets = {};

public:

	FToroSlateStyle()
		: FSlateStyleSet(TEXT("Unknown"))
	{}

	/**
	 * Creates resources and registers a style while retaining shared ownership.
	 * Names already tracked here or present in the Slate registry are logged and ignored.
	 * @tparam StyleSet Default-constructible type derived from FToroSlateStyle.
	 */
	template <typename StyleSet>
	static void Register()
	{
		static_assert(TIsDerivedFrom<StyleSet, FToroSlateStyle>::Value,
			"StyleSet must derive from FToroSlateStyle");

		TSharedPtr<FToroSlateStyle> Style = MakeShared<StyleSet>();
		if (StyleSets.Contains(Style->GetFName()) || FSlateStyleRegistry::FindSlateStyle(Style->GetFName()))
		{
			UE_LOG(LogToroCoreEd, Warning,
				TEXT("FToroSlateStyle: Attempting to register multiple Slate Styles with name %s"), *Style->GetName()
			);

			Style.Reset();
			return;
		}

		Style->AddResources();
		FSlateStyleRegistry::RegisterSlateStyle(*Style);
		StyleSets.Add(Style->GetFName(), Style);

		UE_LOG(LogToroCoreEd, Display, TEXT("FToroSlateStyle: Registered Slate Style with name %s"), *Style->GetName());
	}

	/**
	 * Unregisters all tracked styles and releases this helper's shared ownership.
	 */
	static void UnregisterAll()
	{
		if (!StyleSets.IsEmpty())
		{
			for (TPair<FName, TSharedPtr<FToroSlateStyle>>& Style : StyleSets)
			{
				if (Style.Value.IsValid())
				{
					FSlateStyleRegistry::UnRegisterSlateStyle(*Style.Value);
					Style.Value.Reset();
				}
			}

			StyleSets.Empty();
			UE_LOG(LogToroCoreEd, Display, TEXT("FToroSlateStyle: Unregistered Slate Styles"));
		}
	}

	/** Gets the style name used for Slate registry lookups. */
	FORCEINLINE FName GetFName() const
	{
		return GetStyleSetName();
	}

	/** Gets the style registry name as a string. */
	FORCEINLINE FString GetName() const
	{
		return GetStyleSetName().ToString();
	}

protected:

	static inline const FVector2D Icon20x20 = FVector2D(20.0f, 20.0f);
	static inline const FVector2D Icon24x24 = FVector2D(24.0f, 24.0f);
	static inline const FVector2D Icon32x32 = FVector2D(32.0f, 32.0f);
	static inline const FVector2D Icon64x64 = FVector2D(64.0f, 64.0f);

	/**
	 * Initializes a style with its registry name.
	 * @param InName Unique name used when registering and referencing this style.
	 */
	FToroSlateStyle(const FName& InName)
		: FSlateStyleSet(InName)
	{}

	/**
	 * Adds a style-owned PNG image brush under a resource key.
	 * @param Name Resource key used by Slate widgets and icons.
	 * @param Path Path relative to the style content root, without the .png extension.
	 * @param Size Brush dimensions in Slate units.
	 */
	void AddPNG(const FName& Name, const FString& Path, const FVector2D& Size);

	/**
	 * Adds a style-owned SVG image brush under a resource key.
	 * @param Name Resource key used by Slate widgets and icons.
	 * @param Path Path relative to the style content root, without the .svg extension.
	 * @param Size Brush dimensions in Slate units.
	 */
	void AddSVG(const FName& Name, const FString& Path, const FVector2D& Size);

	/** Populates resources after construction and before the style is registered; set the content root before adding brushes. */
	virtual void AddResources() = 0;
};
