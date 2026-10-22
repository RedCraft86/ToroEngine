// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "ToroEngineEd.h"
#include "Containers/Map.h"
#include "Math/Vector2D.h"
#include "UObject/NameTypes.h"
#include "Styling/SlateStyle.h"
#include "Brushes/SlateImageBrush.h"
#include "Containers/UnrealString.h"
#include "Templates/SharedPointer.h"
#include "Templates/UnrealTypeTraits.h"
#include "Styling/SlateStyleRegistry.h"
#include "Styling/SlateStyleMacros.h"

/**
 * Base for named editor Slate styles whose resources are populated before registration.
 * The shared registry retains each accepted style until UnregisterAll() releases it.
 * Register on the editor game thread and remove consumers before unregistering styles.
 */
class TOROENGINEED_API FToroSlateStyle : protected FSlateStyleSet, public TSharedFromThis<FToroSlateStyle>
{
	/** Owns the styles registered through this class, keyed by their globally unique style names. */
	static inline TMap<FName, TSharedPtr<FToroSlateStyle>> Entries = {};

public:

	/**
	 * Creates a style, populates its resources, and registers it with the global Slate style registry.
	 * @tparam StyleSet Concrete FToroSlateStyle subclass with a constructor accessible to MakeShared().
	 *         The type must be constructible without arguments and supply a non-empty style name.
	 * @return Registered style name, or NAME_None when the name is empty or already registered.
	 */
	template<typename StyleSet>
	static FName Register()
	{
		static_assert(TIsDerivedFrom<StyleSet, FToroSlateStyle>::Value,
			"StyleSet must derive from FToroSlateStyle");

		TSharedPtr<FToroSlateStyle> Style = MakeShared<StyleSet>();
		if (Style->GetFName().IsNone())
		{
			UE_LOG(LogToroEngineEd, Warning, TEXT("FToroSlateStyle: Attempting to register a slate style with an invalid name"));
			Style.Reset();
			return NAME_None;
		}

		if (Entries.Contains(Style->GetFName()) || FSlateStyleRegistry::FindSlateStyle(Style->GetFName()))
		{
			UE_LOG(LogToroEngineEd, Warning,
				TEXT("FToroSlateStyle: Attempting to register multiple slate styles with name %s"), *Style->GetName()
			);

			Style.Reset();
			return NAME_None;
		}

		Style->AddResources();
		FSlateStyleRegistry::RegisterSlateStyle(*Style);
		Entries.Add(Style->GetFName(), Style);

		UE_LOG(LogToroEngineEd, Display, TEXT("FToroSlateStyle: Registered slate style with name %s"), *Style->GetName());
		return Style->GetFName();
	}

	/**
	 * Unregisters all retained styles and releases their shared ownership.
	 * Style destruction deletes the brushes allocated by AddPNG() and AddSVG().
	 * Remove widgets and other consumers of these styles and brushes before calling this during shutdown.
	 * @note Automatically called by ToroEngineEd -> Module Shutdown.
	 */
	static void UnregisterAll()
	{
		if (!Entries.IsEmpty())
		{
			for (TPair<FName, TSharedPtr<FToroSlateStyle>>& Entry : Entries)
			{
				if (Entry.Value.IsValid())
				{
					FSlateStyleRegistry::UnRegisterSlateStyle(Entry.Key);
					Entry.Value.Reset();
				}

				UE_LOG(LogToroEngineEd, Display, TEXT("FToroSlateStyle: Unregistered all slate styles"));
			}

			Entries.Empty();
		}
	}

	/** Returns the style name used as the global registry key. */
	FORCEINLINE FName GetFName() const
	{
		return GetStyleSetName();
	}

	/** Returns the style registry key as a string for logging or diagnostics. */
	FORCEINLINE FString GetName() const
	{
		return GetStyleSetName().ToString();
	}

protected:

	/** Standard 20-by-20 brush size in Slate units. */
	static inline const FVector2D Icon20x20 = FVector2D(20.0f, 20.0f);
	/** Standard 24-by-24 brush size in Slate units. */
	static inline const FVector2D Icon24x24 = FVector2D(24.0f, 24.0f);
	/** Standard 32-by-32 brush size in Slate units. */
	static inline const FVector2D Icon32x32 = FVector2D(32.0f, 32.0f);
	/** Standard 64-by-64 brush size in Slate units. */
	static inline const FVector2D Icon64x64 = FVector2D(64.0f, 64.0f);

	/** Creates an unnamed style, which the static registration API rejects. */
	FToroSlateStyle()
		: FSlateStyleSet(NAME_None)
	{}

	/**
	 * Creates a style with the supplied registry key.
	 * @param InName Non-empty name that must be unique across the global Slate style registry.
	 */
	FToroSlateStyle(const FName& InName)
		: FSlateStyleSet(InName)
	{}

	/**
	 * Allocates a PNG image brush owned by this style.
	 * Set the content root before calling and use a unique brush key to avoid replacing an owned allocation.
	 * @param Name Resource key used by consumers to look up the brush.
	 * @param Path Path relative to the content root, without the automatically appended .png extension.
	 * @param Size Desired brush dimensions in Slate units.
	 */
	void AddPNG(const FName& Name, const FString& Path, const FVector2D& Size)
	{
		Set(Name, new IMAGE_BRUSH(Path, Size));
	}

	/**
	 * Allocates an SVG vector image brush owned by this style.
	 * Set the content root before calling and use a unique brush key to avoid replacing an owned allocation.
	 * @param Name Resource key used by consumers to look up the brush.
	 * @param Path Path relative to the content root, without the automatically appended .svg extension.
	 * @param Size Desired brush dimensions in Slate units.
	 */
	void AddSVG(const FName& Name, const FString& Path, const FVector2D& Size)
	{
		Set(Name, new IMAGE_BRUSH_SVG(Path, Size));
	}

	/**
	 * Populates the style before it is published to the global registry.
	 * Set the content root and add all resource definitions here.
	 * Called once by Register() for each accepted style instance.
	 */
	virtual void AddResources() = 0;
};
