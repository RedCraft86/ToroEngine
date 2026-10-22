// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "ToroEngineEd.h"
#include "Styling/SlateStyleMacros.h"
#include "Styling/SlateStyleRegistry.h"

class TOROENGINEED_API FToroSlateStyle : protected FSlateStyleSet, public TSharedFromThis<FToroSlateStyle>
{
	static inline TMap<FName, TSharedPtr<FToroSlateStyle>> Entries = {};

public:

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
			}

			Entries.Empty();
			UE_LOG(LogToroEngineEd, Display, TEXT("FToroSlateStyle: Unregistered all slate styles"));
		}
	}

	FORCEINLINE FName GetFName() const
	{
		return GetStyleSetName();
	}

	FORCEINLINE FString GetName() const
	{
		return GetStyleSetName().ToString();
	}

protected:

	static inline const FVector2D Icon20x20 = FVector2D(20.0f, 20.0f);
	static inline const FVector2D Icon24x24 = FVector2D(24.0f, 24.0f);
	static inline const FVector2D Icon32x32 = FVector2D(32.0f, 32.0f);
	static inline const FVector2D Icon64x64 = FVector2D(64.0f, 64.0f);

	FToroSlateStyle()
		: FSlateStyleSet(NAME_None)
	{}

	FToroSlateStyle(const FName& InName)
		: FSlateStyleSet(InName)
	{}

	void AddPNG(const FName& Name, const FString& Path, const FVector2D& Size)
	{
		Set(Name, new IMAGE_BRUSH(Path, Size));
	}

	void AddSVG(const FName& Name, const FString& Path, const FVector2D& Size)
	{
		Set(Name, new IMAGE_BRUSH_SVG(Path, Size));
	}

	virtual void AddResources() = 0;
};
