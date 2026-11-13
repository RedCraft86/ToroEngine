// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "UserSettings/ToroGameUserSettings.h"
#include "Engine/Engine.h"

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

void UToroGameUserSettings::Initialize(UGameInstance& GameInst)
{
	if (!GameInstance.IsValid())
	{
		// Do full initialization
		GameInstance = &GameInst;
		LoadSettings(true);
		ValidateSettings();
		if (!ScalabilityBaseline.bSaved)
		{
			bIsFirstLoad = true;

			SetScreenResolution(GetDesktopResolution());
			SetFullscreenMode(EWindowMode::WindowedFullscreen);
			ConfirmVideoMode();

			RunHardwareBenchmark(10, 0.7f, 0.7f);
			ScalabilityBaseline.Capture(ScalabilityQuality);
		}
	}

	ApplySettings(false);
}

UWorld* UToroGameUserSettings::GetWorld() const
{
	return GameInstance.IsValid()
		? GameInstance->GetWorld()
		: Super::GetWorld();
}

void UToroGameUserSettings::RunHardwareBenchmark(int32 WorkScale, float CPUMultiplier, float GPUMultiplier)
{
	Super::RunHardwareBenchmark(WorkScale, CPUMultiplier, GPUMultiplier);
	// Capture any subsequent benchmark runs, they always act as a baseline
	ScalabilityBaseline.Capture(ScalabilityQuality);
	RequestUIUpdate();
}

