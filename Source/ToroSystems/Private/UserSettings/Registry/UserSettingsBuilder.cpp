// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "UserSettings/Registration/UserSettingsBuilder.h"
#include "UserSettings/Registration/UserSettingsRegistry.h"
#include "UserSettings/ToroGameUserSettings.h"
#include "UserSettings/UserSettingsConfig.h"

#define LOCTEXT_NAMESPACE "ToroEngine"

void UUserSettingsBuilder::Build(FUserSettingsRegistry& Registry, UToroGameUserSettings& Settings)
{
	if (const FUserSettingsConfig* Cfg = FUserSettingsConfig::Get())
	{
		if (const TSubclassOf<UUserSettingsBuilder> BuilderClass = Cfg->SettingsBuilder.LoadSynchronous())
		{
			if (const UUserSettingsBuilder* Builder = BuilderClass->GetDefaultObject<UUserSettingsBuilder>())
			{
				Builder->BuildRegistry(Registry, Settings);
				return;
			}
		}
	}

	if (const UUserSettingsBuilder* Builder = GetDefault<UUserSettingsBuilder>())
	{
		Builder->BuildRegistry(Registry, Settings);
	}
}

void UUserSettingsBuilder::BuildRegistry(FUserSettingsRegistry& Registry, UToroGameUserSettings& Settings) const
{
	const TWeakObjectPtr<UToroGameUserSettings> WeakSettings(&Settings);

	using namespace UserSettingsTags;

	auto& GeneralPage = Registry.EditPage(TEXT("General"), LOCTEXT("GeneralPage", "General"));
	GeneralPage.GetMetadata().Description = LOCTEXT("GeneralPageDescription", "Configure camera and other preferences.");
	GeneralPage.GetMetadata().Impact = EUserSettingImpact::None;
	{
		auto& CameraSection = GeneralPage.EditSection(TEXT("Camera"), LOCTEXT("CameraSection", "Camera"));
		CameraSection.GetMetadata().Description = LOCTEXT("CameraSectionDescription", "Adjust camera sensitivity and input direction.");
		CameraSection.GetMetadata().Impact = EUserSettingImpact::None;

		CameraSection.AddSlider(Camera_SensitivityX)
			.DisplayName(LOCTEXT("SensitivityX", "Horizontal Sensitivity"))
			.Description(LOCTEXT("SensitivityXHelp", "Adjust horizontal camera sensitivity."))
			.Default(1.0f)
			.Range(0.2f, 2.0f)
			.Delta(0.1f)
			.NumDecimal(1)
			.Bind(
				&Settings,
				&UToroGameUserSettings::GetSensitivityX,
				&UToroGameUserSettings::SetSensitivityX
			);

		CameraSection.AddSlider(Camera_SensitivityY)
			.DisplayName(LOCTEXT("SensitivityY", "Vertical Sensitivity"))
			.Description(LOCTEXT("SensitivityYHelp", "Adjust vertical camera sensitivity."))
			.Default(1.0f)
			.Range(0.2f, 2.0f)
			.Delta(0.1f)
			.NumDecimal(1)
			.Bind(
				&Settings,
				&UToroGameUserSettings::GetSensitivityY,
				&UToroGameUserSettings::SetSensitivityY
			);

		CameraSection.AddToggle(Camera_InvertMouseX)
			.DisplayName(LOCTEXT("InvertX", "Invert Horizontal Input"))
			.Description(LOCTEXT("InvertXHelp", "Reverse horizontal mouse camera input."))
			.Default(false)
			.Bind(
				&Settings,
				&UToroGameUserSettings::GetInvertMouseX,
				&UToroGameUserSettings::SetInvertMouseX
			);

		CameraSection.AddToggle(Camera_InvertMouseY)
			.DisplayName(LOCTEXT("InvertY", "Invert Vertical Input"))
			.Description(LOCTEXT("InvertYHelp", "Reverse vertical mouse camera input."))
			.Default(false)
			.Bind(
				&Settings,
				&UToroGameUserSettings::GetInvertMouseY,
				&UToroGameUserSettings::SetInvertMouseY
			);
	}

	auto& VideoPage = Registry.EditPage(TEXT("Video"), LOCTEXT("VideoPage", "Video"));
	VideoPage.GetMetadata().Description = LOCTEXT("VideoPageDescription", "Configure display and frame presentation preferences.");
	VideoPage.GetMetadata().Impact = EUserSettingImpact::Medium;
	{
		auto& DisplaySection = VideoPage.EditSection(TEXT("Display"), LOCTEXT("DisplaySection", "Display"));
		DisplaySection.GetMetadata().Description = LOCTEXT("DisplaySectionDescription", "Adjust brightness, synchronization, and the frame rate limit.");
		DisplaySection.GetMetadata().Impact = EUserSettingImpact::Medium;

		// TODO: Handling for Resolution and WindowMode

		DisplaySection.AddToggle(Display_VSync)
			.DisplayName(LOCTEXT("VSync", "V-Sync"))
			.Description(LOCTEXT("VSyncHelp", "Synchronize presentation with the display to reduce tearing."))
			.Default(true)
			.Bind(
				&Settings,
				&UToroGameUserSettings::IsVSyncEnabled,
				&UToroGameUserSettings::SetVSyncEnabled
			);

		DisplaySection.AddSelector<float>(Display_FrameRateLimit)
			.DisplayName(LOCTEXT("FrameLimit", "Frame Rate Limit"))
			.Description(LOCTEXT("FrameLimitHelp", "Limit the frame rate. V-Sync may impose an additional limit."))
			.Default(60.0f)
			.Option(30.0f, LOCTEXT("FPS30", "30"))
			.Option(60.0f, LOCTEXT("FPS60", "60"))
			.Option(120.0f, LOCTEXT("FPS120", "120"))
			.Option(0.0f, LOCTEXT("Unlimited", "Unlimited"))
			.Comparator([](const float& A, const float& B) -> bool {
				return FMath::IsNearlyEqual(A, B, 0.1f);
			})
			.Bind(
				&Settings,
				&UToroGameUserSettings::GetFrameRateLimit,
				&UToroGameUserSettings::SetFrameRateLimit
			);

		DisplaySection.AddSlider(Display_Brightness)
			.DisplayName(LOCTEXT("Brightness", "Brightness"))
			.Description(LOCTEXT("BrightnessHelp", "Adjust image midtones. 50 is neutral."))
			.Type(EUserSettingSliderType::Live)
			.Default(50.0f)
			.Range(20.0f, 80.0f)
			.Delta(1.0f)
			.NumDecimal(0)
			.Bind(
				&Settings,
				&UToroGameUserSettings::GetBrightness,
				&UToroGameUserSettings::SetBrightness
			);
	}

	auto& GraphicsPage = Registry.EditPage(TEXT("Graphics"), LOCTEXT("GraphicsPage", "Graphics"));
	GraphicsPage.GetMetadata().Description = LOCTEXT("GraphicsPageDescription", "Balance rendering quality and performance.");
	GraphicsPage.GetMetadata().Impact = EUserSettingImpact::High;
	{
		const FScalabilityBaseline& Baseline = Settings.GetScalabilityBaseline();
		Scalability::FQualityLevels BaselineQuality;
		Baseline.Restore(BaselineQuality);

		auto& ImageSection = GraphicsPage.EditSection(TEXT("Image"), LOCTEXT("ImageSection", "Image"));
		ImageSection.GetMetadata().Description = LOCTEXT("ImageSectionDescription", "Configure anti-aliasing and internal rendering resolution.");
		ImageSection.GetMetadata().Impact = EUserSettingImpact::High;

		ImageSection.AddSelector<EImageFidelityMode::Type>(Graphics_ImageFidelityMode)
			.DisplayName(LOCTEXT("Fidelity", "Image Fidelity"))
			.Description(LOCTEXT("FidelityHelp", "Choose the anti-aliasing method. TSR can reconstruct a lower-resolution image."))
			.Impact(EUserSettingImpact::Medium)
			.Default(EImageFidelityMode::TAA)
			.Option(EImageFidelityMode::None, LOCTEXT("AANone", "None"))
			.Option(EImageFidelityMode::FXAA, LOCTEXT("FXAA", "FXAA"))
			.Option(EImageFidelityMode::TAA, LOCTEXT("TAA", "TAA"))
			.Option(EImageFidelityMode::TSR, LOCTEXT("TSR", "TSR"))
			.Option(EImageFidelityMode::SMAA, LOCTEXT("SMAA", "SMAA"))
			.Bind(
				&Settings,
				&UToroGameUserSettings::GetImageFidelityMode,
				&UToroGameUserSettings::SetImageFidelityMode
			);

		ImageSection.AddSlider(Graphics_TSRScreenPercentage)
			.DisplayName(LOCTEXT("TSRPercentage", "TSR Screen Percentage"))
			.Description(LOCTEXT("TSRPercentageHelp", "Lower the internal rendering resolution used by TSR."))
			.Impact(EUserSettingImpact::High)
			.Default(100.0f)
			.Range(10.0f, 100.0f)
			.Delta(1.0f)
			.NumDecimal(0)
			.IsEnabled([WeakSettings]() {
				return WeakSettings.IsValid() && WeakSettings->GetImageFidelityMode() == EImageFidelityMode::TSR;
			})
			.Bind(
				&Settings,
				&UToroGameUserSettings::GetTSRScreenPercentage,
				&UToroGameUserSettings::SetTSRScreenPercentage
			);

		float ScaleNormalized, ScaleCurrent, ScaleMin, ScaleMax;
		Settings.GetResolutionScaleInformationEx(ScaleNormalized, ScaleCurrent, ScaleMin, ScaleMax);
		ImageSection.AddSlider(Graphics_ResolutionScale)
			.DisplayName(LOCTEXT("ResolutionScale", "Resolution Scale"))
			.Description(LOCTEXT("ResolutionScaleHelp", "Adjust internal rendering resolution. TSR uses its separate screen percentage."))
			.Impact(EUserSettingImpact::High)
			.Default(FMath::Clamp(Baseline.ResolutionQuality, ScaleMin, ScaleMax))
			.Range(ScaleMin, ScaleMax)
			.Delta(1.0f)
			.NumDecimal(0)
			.IsEnabled([WeakSettings](){
				return WeakSettings.IsValid() && WeakSettings->GetImageFidelityMode() != EImageFidelityMode::TSR;
			})
			.BindPostApply(&Settings, [](UToroGameUserSettings& Target, const FUserSettingEntry&) {
				Target.RequestUIUpdate();
			})
			.BindCallbacks(&Settings,
				[](const UToroGameUserSettings& Target, const FUserSettingEntry&)
				{
					float Normalized, Current, Min, Max;
					Target.GetResolutionScaleInformationEx(Normalized, Current, Min, Max);
					return Current;
				},
				[](UToroGameUserSettings& Target, const FUserSettingEntry&, const float& Value)
				{
					Target.SetResolutionScaleValueEx(Value);
				}
			);

		auto& EffectsSection = GraphicsPage.EditSection(TEXT("Effects"), LOCTEXT("EffectsSection", "Effects"));
		EffectsSection.GetMetadata().Description = LOCTEXT("EffectsSectionDescription", "Configure lighting and visual effects.");
		EffectsSection.GetMetadata().Impact = EUserSettingImpact::High;

		EffectsSection.AddSelector<int32>(Graphics_MotionBlurQuality)
			.DisplayName(LOCTEXT("MotionBlur", "Motion Blur Amount"))
			.Description(LOCTEXT("MotionBlurHelp", "Adjust motion blur strength. This does not change its rendering quality."))
			.Default(2)
			.Option(0, LOCTEXT("BlurOff", "Off"))
			.Option(1, LOCTEXT("BlurLow", "Low"))
			.Option(2, LOCTEXT("BlurMedium", "Medium"))
			.Option(3, LOCTEXT("BlurHigh", "High"))
			.Bind(
				&Settings,
				&UToroGameUserSettings::GetMotionBlurAmount,
				&UToroGameUserSettings::SetMotionBlurAmount
			);

		EffectsSection.AddSelector<ELumenUserSetting::Type>(Graphics_LumenMode)
			.DisplayName(LOCTEXT("Lumen", "Lumen Mode"))
			.Description(LOCTEXT("LumenHelp", "Choose whether Lumen provides global illumination, reflections, or both."))
			.Impact(EUserSettingImpact::High)
			.Default(ELumenUserSetting::Enabled)
			.Option(ELumenUserSetting::Disabled, LOCTEXT("LumenDisabled", "Disabled"))
			.Option(ELumenUserSetting::ReflectionOnly, LOCTEXT("LumenReflection", "Reflections Only"))
			.Option(ELumenUserSetting::LightingOnly, LOCTEXT("LumenLighting", "Lighting Only"))
			.Option(ELumenUserSetting::Enabled, LOCTEXT("LumenEnabled", "Enabled"))
			.Bind(
				&Settings,
				&UToroGameUserSettings::GetLumenMode,
				&UToroGameUserSettings::SetLumenMode
			);

		auto& QualitySection = GraphicsPage.EditSection(TEXT("Scalability"), LOCTEXT("ScalabilitySection", "Scalability"));
		QualitySection.GetMetadata().Description = LOCTEXT("QualitySectionDescription", "Choose rendering quality levels or detect hardware recommendations.");
		QualitySection.GetMetadata().Impact = EUserSettingImpact::High;

		QualitySection.AddAction(Graphics_AutoDetect)
			.DisplayName(LOCTEXT("AutoDetect", "Auto Detect"))
			.Description(LOCTEXT("AutoDetectHelp", "Run the hardware benchmark and update recommended quality levels."))
			.BindCallback(&Settings, [](UToroGameUserSettings& Target, const FUserSettingEntry&) {
				Target.RunHardwareBenchmark(10, 0.7f, 0.7f);
			});

		QualitySection.AddSelector<int32>(Graphics_OverallQuality)
			.DisplayName(LOCTEXT("Overall", "Overall Quality"))
			.Description(LOCTEXT("OverallHelp", "Set all scalability groups to one quality level. Mixed values appear as Custom."))
			.Impact(EUserSettingImpact::High)
			.Default(BaselineQuality.GetSingleQualityLevel())
			.BindReset(&Settings, [](UToroGameUserSettings& Target, const FUserSettingEntry&) {
				Target.RevertAllScalabilitySettings();
			})
			.IsDefault([WeakSettings]()
			{
				if (const UToroGameUserSettings* Target = WeakSettings.Get())
				{
					const FScalabilityBaseline& Saved = Target->GetScalabilityBaseline();

					float Normalized, Current, Min, Max;
					Target->GetResolutionScaleInformationEx(Normalized, Current, Min, Max);

					return FMath::IsNearlyEqual(Current, Saved.ResolutionQuality)
						&& Target->GetViewDistanceQuality() == Saved.ViewDistanceQuality
						&& Target->GetAntiAliasingQuality() == Saved.AntiAliasingQuality
						&& Target->GetShadowQuality() == Saved.ShadowQuality
						&& Target->GetGlobalIlluminationQuality() == Saved.GlobalIlluminationQuality
						&& Target->GetReflectionQuality() == Saved.ReflectionQuality
						&& Target->GetPostProcessingQuality() == Saved.PostProcessQuality
						&& Target->GetTextureQuality() == Saved.TextureQuality
						&& Target->GetVisualEffectQuality() == Saved.EffectsQuality
						&& Target->GetFoliageQuality() == Saved.FoliageQuality
						&& Target->GetShadingQuality() == Saved.ShadingQuality
						&& Target->GetLandscapeQuality() == Saved.LandscapeQuality;
				}

				return false;
			})
			.Option(-1, LOCTEXT("QualityCustom", "Custom"))
			.Option(0, LOCTEXT("QualityLow", "Low"))
			.Option(1, LOCTEXT("QualityMedium", "Medium"))
			.Option(2, LOCTEXT("QualityHigh", "High"))
			.Option(3, LOCTEXT("QualityEpic", "Epic"))
			.Option(4, LOCTEXT("QualityCinematic", "Cinematic"))
			.Bind(
				&Settings,
				&UToroGameUserSettings::GetOverallScalabilityLevel,
				&UToroGameUserSettings::SetOverallScalabilityLevel
			);

		const auto AddQuality = [&Settings, &QualitySection](
			FGameplayTag Key,
			FText Name,
			FText Description,
			int32 DefaultValue,
			auto Getter,
			auto Setter,
			EUserSettingImpact Impact
		)
		{
			QualitySection.AddSelector<int32>(Key)
				.DisplayName(MoveTemp(Name))
				.Description(MoveTemp(Description))
				.Impact(Impact)
				.Default(DefaultValue)
				.Option(0, LOCTEXT("QualityLow", "Low"))
				.Option(1, LOCTEXT("QualityMedium", "Medium"))
				.Option(2, LOCTEXT("QualityHigh", "High"))
				.Option(3, LOCTEXT("QualityEpic", "Epic"))
				.Option(4, LOCTEXT("QualityCinematic", "Cinematic"))
				.BindPostApply(&Settings, [](UToroGameUserSettings& Target, const FUserSettingEntry&) {
					Target.RequestUIUpdate();
				})
				.BindCallbacks(
					&Settings,
					[Getter](const UToroGameUserSettings& Target, const FUserSettingEntry&)
					{
						return (Target.*Getter)();
					},
					[Setter](UToroGameUserSettings& Target, const FUserSettingEntry&, const int32& Value)
					{
						(Target.*Setter)(Value);
					}
				);
		};

		AddQuality(
			Graphics_ViewDistanceQuality,
			LOCTEXT("ViewDistance", "View Distance"),
			LOCTEXT("ViewDistanceHelp", "Adjust the distance at which objects are visible."),
			Baseline.ViewDistanceQuality,
			&UToroGameUserSettings::GetViewDistanceQuality,
			&UToroGameUserSettings::SetViewDistanceQuality,
			EUserSettingImpact::Medium
		);

		AddQuality(
			Graphics_AntiAliasingQuality,
			LOCTEXT("AAQuality", "Anti-Aliasing Quality"),
			LOCTEXT("AAQualityHelp", "Adjust the quality of the selected anti-aliasing method."),
			Baseline.AntiAliasingQuality,
			&UToroGameUserSettings::GetAntiAliasingQuality,
			&UToroGameUserSettings::SetAntiAliasingQuality,
			EUserSettingImpact::Medium
		);

		AddQuality(
			Graphics_ShadowQuality,
			LOCTEXT("Shadows", "Shadow Quality"),
			LOCTEXT("ShadowsHelp", "Adjust shadow rendering quality."),
			Baseline.ShadowQuality,
			&UToroGameUserSettings::GetShadowQuality,
			&UToroGameUserSettings::SetShadowQuality,
			EUserSettingImpact::High
		);

		AddQuality(
			Graphics_GlobalIlluminationQuality,
			LOCTEXT("GI", "Global Illumination Quality"),
			LOCTEXT("GIHelp", "Adjust indirect lighting quality."),
			Baseline.GlobalIlluminationQuality,
			&UToroGameUserSettings::GetGlobalIlluminationQuality,
			&UToroGameUserSettings::SetGlobalIlluminationQuality,
			EUserSettingImpact::High
		);

		AddQuality(
			Graphics_ReflectionQuality,
			LOCTEXT("Reflections", "Reflection Quality"),
			LOCTEXT("ReflectionsHelp", "Adjust reflection rendering quality."),
			Baseline.ReflectionQuality,
			&UToroGameUserSettings::GetReflectionQuality,
			&UToroGameUserSettings::SetReflectionQuality,
			EUserSettingImpact::High
		);

		AddQuality(
			Graphics_PostProcessQuality,
			LOCTEXT("PostProcess", "Post-Processing Quality"),
			LOCTEXT("PostProcessHelp", "Adjust post-processing effect quality."),
			Baseline.PostProcessQuality,
			&UToroGameUserSettings::GetPostProcessingQuality,
			&UToroGameUserSettings::SetPostProcessingQuality,
			EUserSettingImpact::Medium
		);

		AddQuality(
			Graphics_TextureQuality,
			LOCTEXT("Textures", "Texture Quality"),
			LOCTEXT("TexturesHelp", "Adjust texture quality and memory use."),
			Baseline.TextureQuality,
			&UToroGameUserSettings::GetTextureQuality,
			&UToroGameUserSettings::SetTextureQuality,
			EUserSettingImpact::Medium
		);

		AddQuality(
			Graphics_EffectsQuality,
			LOCTEXT("Effects", "Effects Quality"),
			LOCTEXT("EffectsHelp", "Adjust visual effect quality."),
			Baseline.EffectsQuality,
			&UToroGameUserSettings::GetVisualEffectQuality,
			&UToroGameUserSettings::SetVisualEffectQuality,
			EUserSettingImpact::High
		);

		AddQuality(
			Graphics_FoliageQuality,
			LOCTEXT("Foliage", "Foliage Quality"),
			LOCTEXT("FoliageHelp", "Adjust foliage quality and density."),
			Baseline.FoliageQuality,
			&UToroGameUserSettings::GetFoliageQuality,
			&UToroGameUserSettings::SetFoliageQuality,
			EUserSettingImpact::Medium
		);

		AddQuality(
			Graphics_ShadingQuality,
			LOCTEXT("Shading", "Shading Quality"),
			LOCTEXT("ShadingHelp", "Adjust material shading quality."),
			Baseline.ShadingQuality,
			&UToroGameUserSettings::GetShadingQuality,
			&UToroGameUserSettings::SetShadingQuality,
			EUserSettingImpact::Medium
		);

		AddQuality(
			Graphics_LandscapeQuality,
			LOCTEXT("Landscape", "Landscape Quality"),
			LOCTEXT("LandscapeHelp", "Adjust landscape rendering quality."),
			Baseline.LandscapeQuality,
			&UToroGameUserSettings::GetLandscapeQuality,
			&UToroGameUserSettings::SetLandscapeQuality,
			EUserSettingImpact::Medium
		);
	}

	auto& AudioPage = Registry.EditPage(TEXT("Audio"), LOCTEXT("AudioPage", "Audio"));
	AudioPage.GetMetadata().Description = LOCTEXT("AudioPageDescription", "Configure audio volume preferences.");
	AudioPage.GetMetadata().Impact = EUserSettingImpact::None;
	{
		auto& VolumeSection = AudioPage.EditSection(TEXT("Volumes"), LOCTEXT("VolumesSection", "Volume"));
		VolumeSection.GetMetadata().Description = LOCTEXT("VolumeSectionDescription", "Adjust master volume and individual audio categories.");
		VolumeSection.GetMetadata().Impact = EUserSettingImpact::None;

		const auto AddVolume = [&Settings, &VolumeSection](FGameplayTag Key, FText Name, FText Description)
		{
			VolumeSection.AddSlider(Key)
				.DisplayName(MoveTemp(Name))
				.Description(MoveTemp(Description))
				.Type(EUserSettingSliderType::Live)
				.Default(100.0f)
				.Range(0.0f, 100.0f)
				.Delta(1.0f)
				.NumDecimal(0)
				.BindCallbacks(&Settings,
					[](const UToroGameUserSettings& Target, const FUserSettingEntry& Entry)
					{
						return Target.GetAudioVolume(Entry.GetKey());
					},
					[](UToroGameUserSettings& Target, const FUserSettingEntry& Entry, const float& Value)
					{
						Target.SetAudioVolume(Entry.GetKey(), Value);
					}
				);
		};

		AddVolume(
			Audio_MasterVolume,
			LOCTEXT("MasterVolume", "Master"),
			LOCTEXT("MasterVolumeHelp", "Adjust the volume of all audio.")
		);

		AddVolume(
			Audio_MusicVolume,
			LOCTEXT("MusicVolume", "Music"),
			LOCTEXT("MusicVolumeHelp", "Adjust music volume.")
		);

		AddVolume(
			Audio_EffectsVolume,
			LOCTEXT("EffectsVolume", "Effects"),
			LOCTEXT("EffectsVolumeHelp", "Adjust sound effect volume.")
		);

		AddVolume(
			Audio_DialogueVolume,
			LOCTEXT("DialogueVolume", "Dialogue"),
			LOCTEXT("DialogueVolumeHelp", "Adjust dialogue volume.")
		);

		AddVolume(
			Audio_UIVolume,
			LOCTEXT("UIVolume", "User Interface"),
			LOCTEXT("UIVolumeHelp", "Adjust interface sound volume.")
		);
	}

}
#undef LOCTEXT_NAMESPACE
