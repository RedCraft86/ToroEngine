// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "UserSettings/ToroGameUserSettings.h"
#include "UserInterface/Widgets/ToroDialogWidget.h"
#include "PostProcess/WorldPostProcessComponent.h"
#include "UserSettings/UserSettingsConfig.h"
#include "Framework/ToroWorldSettings.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Libraries/ToroCVarLibrary.h"
#include "Utilities/RobAccess.h"
#include "AudioModulationStatics.h"
#include "SoundControlBusMix.h"
#include "SoundControlBus.h"
#include "Engine/Engine.h"

namespace 
{
	float FixupFloatValue(float Value, float Min, float Max, float Fallback)
	{
		return FMath::IsFinite(Value) ? FMath::Clamp(Value, Min, Max) : Fallback;
	}
}

ROB_DEFINE_VAR(
	UGameUserSettings,
	OnGameUserSettingsUINeedsUpdate,
	FOnGameUserSettingsUINeedsUpdate
);

#define LOCTEXT_NAMESPACE "ToroEngine"
void FScalabilityBaseline::Capture(const Scalability::FQualityLevels& Quality)
{
	bSaved = true;
	ResolutionQuality = Quality.ResolutionQuality;
	ViewDistanceQuality = Quality.ViewDistanceQuality;
	AntiAliasingQuality = Quality.AntiAliasingQuality;
	ShadowQuality = Quality.ShadowQuality;
	GlobalIlluminationQuality = Quality.GlobalIlluminationQuality;
	ReflectionQuality = Quality.ReflectionQuality;
	PostProcessQuality = Quality.PostProcessQuality;
	TextureQuality = Quality.TextureQuality;
	EffectsQuality = Quality.EffectsQuality;
	FoliageQuality = Quality.FoliageQuality;
	ShadingQuality = Quality.ShadingQuality;
	LandscapeQuality = Quality.LandscapeQuality;
}

void FScalabilityBaseline::Restore(Scalability::FQualityLevels& Quality) const
{
	Quality.ResolutionQuality = ResolutionQuality;
	Quality.ViewDistanceQuality = ViewDistanceQuality;
	Quality.AntiAliasingQuality = AntiAliasingQuality;
	Quality.ShadowQuality = ShadowQuality;
	Quality.GlobalIlluminationQuality = GlobalIlluminationQuality;
	Quality.ReflectionQuality = ReflectionQuality;
	Quality.PostProcessQuality = PostProcessQuality;
	Quality.TextureQuality = TextureQuality;
	Quality.EffectsQuality = EffectsQuality;
	Quality.FoliageQuality = FoliageQuality;
	Quality.ShadingQuality = ShadingQuality;
	Quality.LandscapeQuality = LandscapeQuality;
}

UToroGameUserSettings::UToroGameUserSettings()
{
	UToroGameUserSettings::SetToDefaults();
}

UGameUserSettings* UToroGameUserSettings::Get(TSubclassOf<UGameUserSettings> Class)
{
	if (IsRunningDedicatedServer())
	{
		return nullptr;
	}

	UGameUserSettings* GUS = GEngine ? GEngine->GetGameUserSettings() : nullptr;
	return IsValid(GUS) && (!IsValid(Class) || GUS->IsA(Class)) ? GUS : nullptr;
}

void UToroGameUserSettings::RevertAllScalabilitySettings()
{
	ScalabilityBaseline.Restore(ScalabilityQuality);
	RequestUIUpdate();
}

void UToroGameUserSettings::SetSensitivityX(float Value)
{
	SensitivityX = FixupFloatValue(Value, 0.2f, 2.0f, 1.0f);
}

float UToroGameUserSettings::GetSensitivityX() const
{
	return SensitivityX;
}

void UToroGameUserSettings::SetInvertMouseX(bool Value)
{
	bInvertMouseX = Value;
}

bool UToroGameUserSettings::GetInvertMouseX() const
{
	return bInvertMouseX;
}

void UToroGameUserSettings::SetSensitivityY(float Value)
{
	SensitivityY = FixupFloatValue(Value, 0.2f, 2.0f, 1.0f);
}

float UToroGameUserSettings::GetSensitivityY() const
{
	return SensitivityY;
}

void UToroGameUserSettings::SetInvertMouseY(bool Value)
{
	bInvertMouseY = Value;
}

bool UToroGameUserSettings::GetInvertMouseY() const
{
	return bInvertMouseY;
}

void UToroGameUserSettings::SetBrightness(float Value)
{
	Brightness = FixupFloatValue(Value, 20.0f, 80.0f, 50.0f);
	ApplyBrightness();
}

float UToroGameUserSettings::GetBrightness() const
{
	return Brightness;
}

void UToroGameUserSettings::SetMotionBlurAmount(int32 Value)
{
	MotionBlurAmount = FMath::Clamp(Value, 0, 3);
}

int32 UToroGameUserSettings::GetMotionBlurAmount() const
{
	return MotionBlurAmount;
}

void UToroGameUserSettings::SetLumenMode(TEnumAsByte<ELumenUserSetting::Type> Value)
{
	LumenMode = Value;
	ApplyLumenSettings();
}

TEnumAsByte<ELumenUserSetting::Type> UToroGameUserSettings::GetLumenMode() const
{
	return LumenMode;
}

void UToroGameUserSettings::SetImageFidelityMode(TEnumAsByte<EImageFidelityMode::Type> Value)
{
	ImageFidelityMode = Value;
	ApplyFidelitySettings();
	RequestUIUpdate();
}

TEnumAsByte<EImageFidelityMode::Type> UToroGameUserSettings::GetImageFidelityMode() const
{
	return ImageFidelityMode;
}

void UToroGameUserSettings::SetTSRScreenPercentage(float Value)
{
	TSRScreenPercentage = FixupFloatValue(Value, 10.0f, 100.0f, 100.0f);
	ApplyFidelitySettings();
}

float UToroGameUserSettings::GetTSRScreenPercentage() const
{
	return TSRScreenPercentage;
}

void UToroGameUserSettings::SetAudioVolume(FGameplayTag Channel, float Value)
{
	if (!UserSettingsTags::IsCategory(Channel, UserSettingsTags::Audio))
	{
		return;
	}

	const float SanitizedValue = FixupFloatValue(Value, 0.0f, 100.0f, 100.0f);
	if (FUserAudioVolume* FoundVolume = AudioVolumes.FindByPredicate(
		[Channel](const FUserAudioVolume& Flag)
		{
			return Flag.Channel == Channel;
		}))
	{
		FoundVolume->Volume = SanitizedValue;
	}
	else
	{
		AudioVolumes.Emplace(Channel, SanitizedValue);
	}

	ApplyAudioVolume(Channel);
}

float UToroGameUserSettings::GetAudioVolume(FGameplayTag Channel) const
{
	if (!UserSettingsTags::IsCategory(Channel, UserSettingsTags::Audio))
	{
		return 0.0f;
	}

	if (const FUserAudioVolume* FoundVolume = AudioVolumes.FindByPredicate(
		[Channel](const FUserAudioVolume& Flag)
		{
			return Flag.Channel == Channel;
		}))
	{
		return FoundVolume->Volume;
	}

	return 100.0f;
}

void UToroGameUserSettings::AddUserFlagEntry(const FString Key)
{
	FUserDebugFlag DebugFlag(Key);
	if (!DebugFlag.IsValid())
	{
		return;
	}

	if (!DebugFlags.ContainsByPredicate(
		[DebugFlag](const FUserDebugFlag& Flag)
		{
			return Flag.Key == DebugFlag.Key;
		}))
	{
		DebugFlags.Emplace(DebugFlag);
	}
}

void UToroGameUserSettings::SetUserFlagEnabled(const FString Key, bool bEnabled)
{
	FUserDebugFlag DebugFlag(Key, bEnabled);
	if (!DebugFlag.IsValid())
	{
		return;
	}

	if (FUserDebugFlag* FoundFlag = DebugFlags.FindByPredicate(
		[DebugFlag](const FUserDebugFlag& Flag)
		{
			return Flag.Key == DebugFlag.Key;
		}))
	{
		FoundFlag->bEnabled = bEnabled;
		FoundFlag->ApplyCVar();
		return;
	}

	DebugFlags.Emplace_GetRef(DebugFlag).ApplyCVar();
}

bool UToroGameUserSettings::IsUserFlagEnabled(const FString Key) const
{
	FUserDebugFlag DebugFlag(Key);
	if (!DebugFlag.IsValid())
	{
		return false;
	}

	if (const FUserDebugFlag* FoundFlag = DebugFlags.FindByPredicate(
		[DebugFlag](const FUserDebugFlag& Flag)
		{
			return Flag.Key == DebugFlag.Key;
		}))
	{
		return FoundFlag->bEnabled;
	}

	return DebugFlag.bEnabled;
}

void UToroGameUserSettings::Initialize(UGameInstance& GameInst)
{
	if (!GameInstance.IsValid())
	{
		GameInstance = &GameInst;

		// Generate these options here in case it lags
		GenerateResolutionOptions();

		LoadSettings(true);
		ValidateSettings();
		if (!ScalabilityBaseline.bSaved)
		{
			bIsFirstLoad = true;

			const FResolutionOption& Option = GetDefaultResolutionOption();
			SetScreenResolution(Option.Key);
			SetFullscreenMode(Option.Value);
			ConfirmVideoMode();

			RunHardwareBenchmark(10, 0.7f, 0.7f);
			ScalabilityBaseline.Capture(ScalabilityQuality);
		}
	}

	WorldSettings = AToroWorldSettings::Get(&GameInst);

	if (const FUserSettingsConfig* Config = FUserSettingsConfig::Get())
	{
		USoundControlBusMix* Mix = Config->AudioMix.LoadSynchronous();
		if (IsValid(Mix) && !UAudioModulationStatics::IsControlBusMixActive(&GameInst, Mix))
		{
			UAudioModulationStatics::ActivateBusMix(&GameInst, Mix);
		}
	}

	ApplySettings(false);
}

UToroGameUserSettings::FResolutionOption UToroGameUserSettings::GetResolutionOption() const
{
	return {GetScreenResolution(), GetFullscreenMode()};
}

void UToroGameUserSettings::SetResolutionOption(const FResolutionOption& InOption)
{
	SetScreenResolution(InOption.Key);
	SetFullscreenMode(InOption.Value);
	ApplyResolutionSettings(false);

	auto* Response = UToroDialogWidget::Create(this,
		LOCTEXT("ResolutionConfirmTitle", "Accept Resolution Setting?"),
		LOCTEXT("ResolutionConfirmDescription",
			"Do you wish to accept the current settings?\n"
			"This will automatically revert after 5 seconds."),
		{
			LOCTEXT("ResolutionRevert", "Revert"),
			LOCTEXT("ResolutionAccept", "Accept")
		},
		true,
		0
	);
	
	if (Response)
	{
		Response->BindUObject(this, &UToroGameUserSettings::OnResolutionDialogClosed);
	}
	else
	{
		OnResolutionDialogClosed(INDEX_NONE);
	}
}

const UToroGameUserSettings::FResolutionOption& UToroGameUserSettings::GetDefaultResolutionOption() const
{
	static FResolutionOption Resolution {GetDesktopResolution(), EWindowMode::WindowedFullscreen};
	return Resolution;
}

const UToroGameUserSettings::FResolutionOptions& UToroGameUserSettings::GenerateResolutionOptions() const
{
	static FResolutionOptions Resolutions;
	if (Resolutions.IsEmpty())
	{
		TArray<FIntPoint> FullscreenResolutions;
		UKismetSystemLibrary::GetSupportedFullscreenResolutions(FullscreenResolutions);
		for (const FIntPoint& Res : FullscreenResolutions)
		{
			Resolutions.AddUnique({Res, EWindowMode::Fullscreen});
		}

		TArray<FIntPoint> WindowedResolutions;
		UKismetSystemLibrary::GetConvenientWindowedResolutions(WindowedResolutions);
		for (const FIntPoint& Res : FullscreenResolutions)
		{
			Resolutions.AddUnique({Res, EWindowMode::Windowed});
		}

		Resolutions.AddUnique(GetDefaultResolutionOption());

		Resolutions.Sort([](const auto& A, const auto& B) -> bool
		{
			const int64 PixelsA = static_cast<int64>(A.Key.X) * static_cast<int64>(A.Key.Y);
			const int64 PixelsB = static_cast<int64>(B.Key.X) * static_cast<int64>(B.Key.Y);
			if (PixelsA != PixelsB)
			{
				return PixelsA > PixelsB;
			}

			if (A.Key.X != B.Key.X)
			{
				return A.Key.X > B.Key.X;
			}

			if (A.Key.Y != B.Key.Y)
			{
				return A.Key.Y > B.Key.Y;
			}

			return A.Value < B.Value;
		});
	}

	return Resolutions;
}

UWorld* UToroGameUserSettings::GetWorld() const
{
	return GameInstance.IsValid()
		? GameInstance->GetWorld()
		: Super::GetWorld();
}

void UToroGameUserSettings::SetToDefaults()
{
	Super::SetToDefaults();

	bUseVSync = true;
	FrameRateLimit = 60.0f;

	SensitivityX = 1.0f;
	bInvertMouseX = false;
	SensitivityY = 1.0f;
	bInvertMouseY = false;
	Brightness = 50.0f;
	MotionBlurAmount = 2;
	LumenMode = ELumenUserSetting::Enabled;
	ImageFidelityMode = EImageFidelityMode::TAA;
	TSRScreenPercentage = 100.0f;
	AudioVolumes.Reset();
	DebugFlags.Reset();
}

void UToroGameUserSettings::ValidateSettings()
{
	Super::ValidateSettings();

	SensitivityX = FixupFloatValue(SensitivityX, 0.2f, 2.0f, 1.0f);
	SensitivityY = FixupFloatValue(SensitivityY, 0.2f, 2.0f, 1.0f);
	Brightness = FixupFloatValue(Brightness, 20.0f, 80.0f, 50.0f);
	MotionBlurAmount = FMath::Clamp(MotionBlurAmount, 0, 3);
	LumenMode = ELumenUserSetting::FixupValue(LumenMode);
	ImageFidelityMode = EImageFidelityMode::FixupValue(ImageFidelityMode);
	TSRScreenPercentage = FixupFloatValue(TSRScreenPercentage, 10.0f, 100.0f, 100.0f);

	for (auto It = AudioVolumes.CreateIterator(); It; ++It)
	{
		if (!UserSettingsTags::IsCategory(It->Channel, UserSettingsTags::Audio))
		{
			It.RemoveCurrentSwap();
		}
		else
		{
			It->Volume = FixupFloatValue(It->Volume, 0.0f, 100.0f, 100.0f);
		}
	}

	for (auto It = DebugFlags.CreateIterator(); It; ++It)
	{
		if (!It->IsValid())
		{
			It.RemoveCurrentSwap();
		}
	}

	// Limit frame rate values to 0 (unlimited), 30, 60, and 120
	{
		float SnappedLimit = 0.0f; // Invalid, so uncapped
		const float RequestedLimit = GetFrameRateLimit();
		if (FMath::IsFinite(RequestedLimit) && RequestedLimit > 0.0f && RequestedLimit <= 150.0f)
		{
			if (RequestedLimit <= 45.0f)
			{
				SnappedLimit = 30.0f;
			}
			else if (RequestedLimit <= 90.0f)
			{
				SnappedLimit = 60.0f;
			}
			else
			{
				SnappedLimit = 120.0f;
			}
		}
		SetFrameRateLimit(SnappedLimit);
	}
}

void UToroGameUserSettings::ApplyNonResolutionSettings()
{
	Super::ApplyNonResolutionSettings();

	ApplyBrightness();
	ApplyMotionBlur();
	ApplyLumenSettings();
	ApplyFidelitySettings();

	if (const FUserSettingsConfig* Config = FUserSettingsConfig::Get())
	{
		for (const auto& Channel : Config->AudioChannels)
		{
			ApplyAudioVolume(Channel.Key);
		}
	}

	for (const FUserDebugFlag& Flag : DebugFlags)
	{
		Flag.ApplyCVar();
	}

	OnSettingsAppliedEvent.Broadcast();
}

void UToroGameUserSettings::RunHardwareBenchmark(int32 WorkScale, float CPUScale, float GPUScale)
{
	Super::RunHardwareBenchmark(WorkScale, CPUScale, GPUScale);
	// Capture any subsequent benchmark runs, they always act as a baseline
	ScalabilityBaseline.Capture(ScalabilityQuality);
	RequestUIUpdate();
}

void UToroGameUserSettings::SetOverallScalabilityLevel(int32 Value)
{
	if (Value == -1)
	{
		return;
	}

	Super::SetOverallScalabilityLevel(Value);
	RequestUIUpdate();
}

FOnGameUserSettingsUINeedsUpdate& UToroGameUserSettings::GetOnUIUpdate()
{
	return this->*RobAccess(UGameUserSettings, OnGameUserSettingsUINeedsUpdate);
}

FSimpleMulticastDelegate::RegistrationType& UToroGameUserSettings::GetOnSettingsApplied()
{
	return OnSettingsAppliedEvent;
}

void UToroGameUserSettings::ApplyBrightness() const
{
	static IConsoleVariable* ColorMid = UToroCVarLibrary::FindCVar(TEXT("r.Color.Mid"));
	ColorMid->SetWithCurrentPriority(Brightness * 0.01f);
}

void UToroGameUserSettings::ApplyMotionBlur() const
{
	static IConsoleVariable* MBAmount = UToroCVarLibrary::FindCVar(TEXT("r.MotionBlur.Amount"));

	const float MotionBlurValue = static_cast<float>(MotionBlurAmount) * 0.25f;
	if (UWorldPostProcessComponent* WPP = WorldSettings.IsValid() ? WorldSettings->GetPostProcess() : nullptr)
	{
		WPP->Settings.bOverride_MotionBlurAmount = true;
		WPP->Settings.MotionBlurAmount = MotionBlurValue;
		MBAmount->SetWithCurrentPriority(-1.0f);
	}
	else
	{
		MBAmount->SetWithCurrentPriority(MotionBlurValue);
	}
}

void UToroGameUserSettings::ApplyLumenSettings() const
{
	if (UWorldPostProcessComponent* WPP = WorldSettings.IsValid() ? WorldSettings->GetPostProcess() : nullptr)
	{
		WPP->Settings.bOverride_DynamicGlobalIlluminationMethod = true;
		WPP->Settings.DynamicGlobalIlluminationMethod = ELumenUserSetting::SupportsGI(LumenMode)
			? EDynamicGlobalIlluminationMethod::Lumen : EDynamicGlobalIlluminationMethod::None;

		WPP->Settings.bOverride_ReflectionMethod = true;
		WPP->Settings.ReflectionMethod = ELumenUserSetting::SupportsReflections(LumenMode)
			? EReflectionMethod::Lumen : EReflectionMethod::ScreenSpace;
	}
	else
	{
		static IConsoleVariable* GIMethod = UToroCVarLibrary::FindCVar(TEXT("r.DynamicGlobalIlluminationMethod"));
		GIMethod->SetWithCurrentPriority(ELumenUserSetting::SupportsGI(LumenMode)
			? static_cast<int32>(EDynamicGlobalIlluminationMethod::Lumen)
			: static_cast<int32>(EDynamicGlobalIlluminationMethod::None)
		);

		static IConsoleVariable* ReflectMethod = UToroCVarLibrary::FindCVar(TEXT("r.ReflectionMethod"));
		ReflectMethod->SetWithCurrentPriority(ELumenUserSetting::SupportsReflections(LumenMode)
			? static_cast<int32>(EReflectionMethod::Lumen)
			: static_cast<int32>(EReflectionMethod::ScreenSpace)
		);
	}
}

void UToroGameUserSettings::ApplyFidelitySettings() const
{
	static IConsoleVariable* AAMethod = UToroCVarLibrary::FindCVar(TEXT("r.AntiAliasingMethod"));
	AAMethod->SetWithCurrentPriority(static_cast<int32>(EImageFidelityMode::ToNative(ImageFidelityMode)));

	static IConsoleVariable* ScreenPercent = UToroCVarLibrary::FindCVar(TEXT("r.ScreenPercentage"));
	ScreenPercent->SetWithCurrentPriority(ImageFidelityMode == EImageFidelityMode::TSR
			? TSRScreenPercentage
			: ScalabilityQuality.ResolutionQuality
	);
}

void UToroGameUserSettings::ApplyAudioVolume(const FGameplayTag& Channel) const
{
	if (!GetWorld() || !UserSettingsTags::IsCategory(Channel, UserSettingsTags::Audio))
	{
		return;
	}

	const FUserSettingsConfig* Config = FUserSettingsConfig::Get();
	USoundControlBusMix* Mix = Config ? Config->AudioMix.LoadSynchronous() : nullptr;
	if (!Config || !IsValid(Mix))
	{
		return;
	}

	const FUserAudioVolume* FoundVolume = AudioVolumes.FindByPredicate(
		[Channel](const FUserAudioVolume& Flag)
		{
			return Flag.Channel == Channel;
		}
	);

	const float Volume = FoundVolume ? FoundVolume->Volume : 100.0f;
	if (USoundControlBus* Bus = Config->GetAudioChannel(Channel))
	{
		TArray<FSoundControlBusMixStage> Stages;
		Stages.Emplace(Bus, FMath::Clamp(Volume * 0.01f, 0.0f, 1.0f));
		UAudioModulationStatics::UpdateMix(this, Mix, MoveTemp(Stages), 0.0f);
	}
}

void UToroGameUserSettings::OnResolutionDialogClosed(int32 ButtonIndex)
{
	if (ButtonIndex == 1)
	{
		ConfirmVideoMode();
	}
	else
	{
		RevertVideoMode();
		ApplyResolutionSettings(false);
		RequestUIUpdate();
	}
}
#undef LOCTEXT_NAMESPACE
