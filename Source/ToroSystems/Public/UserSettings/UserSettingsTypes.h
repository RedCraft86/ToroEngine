// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "NativeGameplayTags.h"
#include "Engine/EngineTypes.h"
#include "UserSettingsTypes.generated.h"

namespace UserSettingFlags
{
	inline constexpr TCHAR ShowFrameRate[]		= TEXT("r.Debug.ShowFrameRate");
	inline constexpr TCHAR ShowInvisibleWalls[]	= TEXT("r.Debug.ShowInvisibleWalls");

	inline TArray<FString> AllToggleable = {
		ShowFrameRate, ShowInvisibleWalls
	};
}

namespace UserSettingsTags
{
	TOROSYSTEMS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Base);

	// Camera
	TOROSYSTEMS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Camera);
	TOROSYSTEMS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Camera_SensitivityX);
	TOROSYSTEMS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Camera_SensitivityY);
	TOROSYSTEMS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Camera_InvertMouseX);
	TOROSYSTEMS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Camera_InvertMouseY);
	TOROSYSTEMS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Camera_Smoothing);

	// Display
	TOROSYSTEMS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Display);
	TOROSYSTEMS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Display_Resolution);
	TOROSYSTEMS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Display_VSync);
	TOROSYSTEMS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Display_FrameRateLimit);
	TOROSYSTEMS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Display_Brightness);

	// Graphics
	TOROSYSTEMS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Graphics);
	TOROSYSTEMS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Graphics_ImageFidelityMode);
	TOROSYSTEMS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Graphics_ResolutionScale);
	TOROSYSTEMS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Graphics_TSRScreenPercentage);
	TOROSYSTEMS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Graphics_LumenMode);
	TOROSYSTEMS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Graphics_LumenRayLightingMode);
	TOROSYSTEMS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Graphics_MotionBlurQuality);
	TOROSYSTEMS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Graphics_AutoDetect);
	TOROSYSTEMS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Graphics_OverallQuality);
	TOROSYSTEMS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Graphics_ViewDistanceQuality);
	TOROSYSTEMS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Graphics_AntiAliasingQuality);
	TOROSYSTEMS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Graphics_ShadowQuality);
	TOROSYSTEMS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Graphics_GlobalIlluminationQuality);
	TOROSYSTEMS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Graphics_ReflectionQuality);
	TOROSYSTEMS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Graphics_PostProcessQuality);
	TOROSYSTEMS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Graphics_TextureQuality);
	TOROSYSTEMS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Graphics_EffectsQuality);
	TOROSYSTEMS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Graphics_FoliageQuality);
	TOROSYSTEMS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Graphics_ShadingQuality);
	TOROSYSTEMS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Graphics_LandscapeQuality);

	// Audio
	TOROSYSTEMS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Audio);
	TOROSYSTEMS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Audio_MasterVolume);
	TOROSYSTEMS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Audio_UIVolume);
	TOROSYSTEMS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Audio_MusicVolume);
	TOROSYSTEMS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Audio_EffectsVolume);
	TOROSYSTEMS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Audio_DialogueVolume);

	inline bool IsValid(const FGameplayTag& InTag)
	{
		return InTag.IsValid()
			&& InTag.MatchesTag(Base.GetTag())
			&& InTag != Base.GetTag()
			&& InTag != Camera.GetTag()
			&& InTag != Display.GetTag()
			&& InTag != Graphics.GetTag()
			&& InTag != Audio.GetTag();
	}

	inline bool IsCategory(const FGameplayTag& InTag, const FNativeGameplayTag& Category)
	{
		return InTag.IsValid()
			&& InTag != Category.GetTag()
			&& InTag.MatchesTag(Category.GetTag());
	}
}

UENUM(BlueprintType)
enum class ELumenRayLightingSetting : uint8
{
	SurfaceCache				= 0,
	HitLighting					= 1,
	HitLightingForReflections	= 2
};

UENUM(BlueprintType)
namespace ELumenUserSetting
{
	enum Type : uint8
	{
		Disabled		= 0,
		ReflectionOnly	= 1,
		LightingOnly	= 2,
		Enabled			= 3
	};
}
namespace ELumenUserSetting
{
	inline Type FixupValue(Type InType)
	{
		if (static_cast<uint8>(InType) <= ELumenUserSetting::Enabled)
		{
			return InType;
		}

		return ELumenUserSetting::Enabled;
	}

	inline bool SupportsGI(Type InMode)
	{
		return InMode == ELumenUserSetting::LightingOnly
			|| InMode == ELumenUserSetting::Enabled;
	}

	inline bool SupportsReflections(Type InMode)
	{
		return InMode == ELumenUserSetting::ReflectionOnly
			|| InMode == ELumenUserSetting::Enabled;
	}
}

UENUM(BlueprintType)
namespace EImageFidelityMode
{
	enum Type : uint8
	{
		None = 0,
		FXAA = 1,
		TAA  = 2,
		TSR  = 3,
		SMAA = 4
	};
}
namespace EImageFidelityMode
{
	inline Type FixupValue(Type InType)
	{
		if (static_cast<uint8>(InType) <= EImageFidelityMode::SMAA)
		{
			return InType;
		}

		return EImageFidelityMode::TAA;
	}

	inline EAntiAliasingMethod ToNative(Type InMode)
	{
		switch (InMode)
		{
		case EImageFidelityMode::None:
			return AAM_None;

		case EImageFidelityMode::FXAA:
			return AAM_FXAA;

		case EImageFidelityMode::TAA:
			return AAM_TemporalAA;

		case EImageFidelityMode::TSR:
			return AAM_TSR;

		case EImageFidelityMode::SMAA:
			return AAM_SMAA;

		default: // Third party ones like DLSS typically want TSR active
			return AAM_TSR;
		}
	}

	inline Type FromNative(EAntiAliasingMethod InMethod)
	{
		switch (InMethod)
		{
		case AAM_None:
			return EImageFidelityMode::None;

		case AAM_FXAA:
			return EImageFidelityMode::FXAA;

		case AAM_TemporalAA:
			return EImageFidelityMode::TAA;

		case AAM_TSR:
			return EImageFidelityMode::TSR;

		case AAM_SMAA:
			return EImageFidelityMode::SMAA;

		default: // This is a weird case, it won't be able to recover from an invalid
			return EImageFidelityMode::None;
		}
	}
}

USTRUCT(BlueprintInternalUseOnly)
struct TOROSYSTEMS_API FUserAudioVolume final
{
	GENERATED_BODY()

	UPROPERTY() FGameplayTag Channel;
	UPROPERTY() float Volume;

	FUserAudioVolume()
		: Channel(FGameplayTag::EmptyTag)
		, Volume(0.0f)
	{}

	FUserAudioVolume(const FGameplayTag& InChannel, float InVolume)
		: Channel(InChannel)
		, Volume(InVolume)
	{}
};
