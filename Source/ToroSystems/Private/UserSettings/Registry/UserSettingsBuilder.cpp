// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "UserSettings/Registration/UserSettingsBuilder.h"
#include "UserSettings/Registration/UserSettingsRegistry.h"
#include "UserSettings/ToroGameUserSettings.h"
#include "UserSettings/UserSettingsConfig.h"
#include "Libraries/ToroCVarLibrary.h"

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
	GeneralPage.GetMetadata().Description = LOCTEXT("GeneralPageDescription",
		"Configure camera and other preferences.");
	GeneralPage.GetMetadata().Impact = EUserSettingImpact::None;
	{
		auto& CameraSection = GeneralPage.EditSection(TEXT("Camera"), LOCTEXT("CameraSection", "Camera"));
		CameraSection.GetMetadata().Description = LOCTEXT("CameraSectionDescription",
			"Adjust camera sensitivity and input direction.");
		CameraSection.GetMetadata().Impact = EUserSettingImpact::None;

		CameraSection.AddSlider(Camera_SensitivityX)
			.DisplayName(LOCTEXT("SensitivityX", "Horizontal Sensitivity"))
			.Description(LOCTEXT("SensitivityXHelp",
				"Adjust how quickly the camera turns left and right. Higher values increase sensitivity."))
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
			.Description(LOCTEXT("SensitivityYHelp",
				"Adjust how quickly the camera looks up and down. Higher values increase sensitivity."))
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
			.Description(LOCTEXT("InvertXHelp", "Reverse the camera's response to horizontal mouse movement."))
			.Default(false)
			.Bind(
				&Settings,
				&UToroGameUserSettings::GetInvertMouseX,
				&UToroGameUserSettings::SetInvertMouseX
			);

		CameraSection.AddToggle(Camera_InvertMouseY)
			.DisplayName(LOCTEXT("InvertY", "Invert Vertical Input"))
			.Description(LOCTEXT("InvertYHelp", "Reverse the camera's response to vertical mouse movement."))
			.Default(false)
			.Bind(
				&Settings,
				&UToroGameUserSettings::GetInvertMouseY,
				&UToroGameUserSettings::SetInvertMouseY
			);
	}

	auto& VideoPage = Registry.EditPage(TEXT("Video"), LOCTEXT("VideoPage", "Video"));
	VideoPage.GetMetadata().Description = LOCTEXT("VideoPageDescription",
		"Adjust screen resolution, window mode, brightness, and frame rate.");
	VideoPage.GetMetadata().Impact = EUserSettingImpact::Medium;
	{
		auto& DisplaySection = VideoPage.EditSection(TEXT("Display"), LOCTEXT("DisplaySection", "Display"));
		DisplaySection.GetMetadata().Description = LOCTEXT("DisplaySectionDescription",
			"Adjust brightness, synchronization, and the frame rate limit.");
		DisplaySection.GetMetadata().Impact = EUserSettingImpact::Medium;

		DisplaySection.AddSelector<UToroGameUserSettings::FResolutionOption>(Display_Resolution)
			.DisplayName(LOCTEXT("Resolution", "Resolution"))
			.Description(LOCTEXT("ResolutionHelp",
				"Choose the display resolution and window mode. Higher resolutions provide a sharper image "
				"but may reduce performance."))
			.Impact(EUserSettingImpact::Medium)
			.FromArray(
				Settings.GenerateResolutionOptions(),
				[](const UToroGameUserSettings::FResolutionOption& Option) -> FText
				{
					FText WindowType = LOCTEXT("Unknown", "Unknown");
					switch (Option.Value)
					{
					case EWindowMode::Fullscreen:
						WindowType = LOCTEXT("FullscreenMode", "Fullscreen");
						break;

					case EWindowMode::WindowedFullscreen:
						WindowType = LOCTEXT("BorderlessMode", "Borderless");
						break;

					case EWindowMode::Windowed:
						WindowType = LOCTEXT("WindowedMode", "Windowed");
						break;

					default:
						break;
					}

					FNumberFormattingOptions ComponentFormat;
					ComponentFormat.SetUseGrouping(false);
					return FText::Format(LOCTEXT("ResolutionOption", "{0} x {1} ({2})"),
						FText::AsNumber(Option.Key.X, &ComponentFormat),
						FText::AsNumber(Option.Key.Y, &ComponentFormat),
						WindowType
					);
				}
			)
			.Bind(
				&Settings,
				&UToroGameUserSettings::GetResolutionOption,
				&UToroGameUserSettings::SetResolutionOption
			);

		DisplaySection.AddToggle(Display_VSync)
			.DisplayName(LOCTEXT("VSync", "V-Sync"))
			.Description(LOCTEXT("VSyncHelp",
				"Match frame presentation to your display's refresh cycle to reduce screen tearing. May increase input delay."))
			.Impact(EUserSettingImpact::Medium)
			.Default(true)
			.Bind(
				&Settings,
				&UToroGameUserSettings::IsVSyncEnabled,
				&UToroGameUserSettings::SetVSyncEnabled
			);

		DisplaySection.AddSelector<float>(Display_FrameRateLimit)
			.DisplayName(LOCTEXT("FrameLimit", "Frame Rate Limit"))
			.Description(LOCTEXT("FrameLimitHelp",
				"Set the maximum number of frames rendered per second. A lower limit can reduce power use and heat.\n\n"
				"V-Sync may limit the frame rate further."))
			.Impact(EUserSettingImpact::Medium)
			.Option(30.0f, LOCTEXT("FPS30", "30"))
			.Option(60.0f, LOCTEXT("FPS60", "60"))
			.Option(120.0f, LOCTEXT("FPS120", "120"))
			.Option(0.0f, LOCTEXT("Unlimited", "Unlimited"))
			.Default(60.0f)
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
			.Description(LOCTEXT("BrightnessHelp", "Make the image lighter or darker. 50 uses the default brightness."))
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
	GraphicsPage.GetMetadata().Description = LOCTEXT("GraphicsPageDescription",
		"Balance rendering quality and performance.");
	GraphicsPage.GetMetadata().Impact = EUserSettingImpact::High;
	{
		const FScalabilityBaseline& Baseline = Settings.GetScalabilityBaseline();
		Scalability::FQualityLevels BaselineQuality;
		Baseline.Restore(BaselineQuality);

		auto& ImageSection = GraphicsPage.EditSection(TEXT("Image"), LOCTEXT("ImageSection", "Image"));
		ImageSection.GetMetadata().Description = LOCTEXT("ImageSectionDescription",
			"Configure anti-aliasing and internal rendering resolution.");
		ImageSection.GetMetadata().Impact = EUserSettingImpact::High;

		ImageSection.AddSelector<EImageFidelityMode::Type>(Graphics_ImageFidelityMode)
			.DisplayName(LOCTEXT("Fidelity", "Image Fidelity"))
			.Description(LOCTEXT("FidelityHelp",
				"Choose how jagged edges are smoothed.\n\n"
				"Temporal methods use previous frames to improve stability. TSR can also reconstruct the image from a lower rendering resolution."))
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
			.Description(LOCTEXT("TSRPercentageHelp",
				"Set the rendering resolution used before TSR reconstructs the final image. Lower values can "
				"improve performance but may reduce detail and image stability.\n\n"
				"100 uses the full output resolution."))
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
			.Description(LOCTEXT("ResolutionScaleHelp",
				"Set the rendering resolution relative to the output resolution. Lower values can improve performance but make the image softer.\n\n"
				"When using TSR, adjust TSR Screen Percentage instead."))
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
		EffectsSection.GetMetadata().Description = LOCTEXT("EffectsSectionDescription",
			"Configure lighting and visual effects.");
		EffectsSection.GetMetadata().Impact = EUserSettingImpact::High;

		EffectsSection.AddSelector<int32>(Graphics_MotionBlurQuality)
			.DisplayName(LOCTEXT("MotionBlur", "Motion Blur Amount"))
			.Description(LOCTEXT("MotionBlurHelp",
				"Choose how much moving objects and camera movement blur the image. Off removes motion blur."))
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
			.Description(LOCTEXT("LumenHelp",
				"Enable Lumen for light bouncing between surfaces, reflections, or both. These effects "
				"improve lighting realism but can significantly reduce performance."))
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

		EffectsSection.AddSelector<ELumenRayLightingSetting>(Graphics_LumenRayLightingMode)
			.DisplayName(LOCTEXT("LumenRayLighting", "Lumen Ray Lighting Mode"))
			.Description(LOCTEXT("LumenRayLightingHelp",
				"Requires supported Lumen hardware ray tracing.\n\n"
				"Surface Cache: Best performance.\n"
				"Hit Lighting for Reflections: Higher-quality reflections at increased GPU cost.\n"
				"Hit Lighting: Higher-quality lighting and reflections at significantly increased GPU cost.\n\n"
				"Unsupported configurations may fall back. Reflections-only Lumen always uses Hit Lighting "
				"for Reflections."))
			.Impact(EUserSettingImpact::High)
			.Default(ELumenRayLightingSetting::SurfaceCache)
			.Option(ELumenRayLightingSetting::SurfaceCache, LOCTEXT("RaySurfaceCache", "Surface Cache"))
			.Option(ELumenRayLightingSetting::HitLightingForReflections, LOCTEXT("RayHitReflections", "Hit Lighting for Reflections"))
			.Option(ELumenRayLightingSetting::HitLighting, LOCTEXT("RayHitLighting", "Hit Lighting"))
			.IsEnabled([WeakSettings]() {
				return WeakSettings.IsValid() && WeakSettings->GetLumenMode() != ELumenUserSetting::Disabled;
			})
			.Bind(&Settings,
				&UToroGameUserSettings::GetLumenRayLightingMode,
				&UToroGameUserSettings::SetLumenRayLightingMode);

		auto& QualitySection = GraphicsPage.EditSection(TEXT("Scalability"), LOCTEXT("ScalabilitySection", "Scalability"));
		QualitySection.GetMetadata().Description = LOCTEXT("QualitySectionDescription",
			"Choose a graphics preset or adjust individual effects to balance visual detail and performance.");
		QualitySection.GetMetadata().Impact = EUserSettingImpact::High;

		QualitySection.AddAction(Graphics_AutoDetect)
			.DisplayName(LOCTEXT("AutoDetect", "Auto Detect"))
			.Description(LOCTEXT("AutoDetectHelp",
				"Test your hardware to select recommended graphics quality settings.\n\n"
				"Replaces your current quality choices and updates the settings used when resetting graphics quality."))
			.Impact(EUserSettingImpact::Medium)
			.BindCallback(&Settings, [](UToroGameUserSettings& Target, const FUserSettingEntry&) {
				Target.RunHardwareBenchmark(10, 0.7f, 0.7f);
			});

		QualitySection.AddSelector<int32>(Graphics_OverallQuality)
			.DisplayName(LOCTEXT("Overall", "Overall Quality"))
			.Description(LOCTEXT("OverallHelp",
				"Choose a preset for all graphics quality settings. Higher presets favor visual detail over performance.\n\n"
				"Custom means individual settings use different quality levels."))
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
			LOCTEXT("ViewDistanceHelp",
				"Adjust how far away distance-limited objects remain visible. Higher settings show more distant detail but can reduce performance."),
			Baseline.ViewDistanceQuality,
			&UToroGameUserSettings::GetViewDistanceQuality,
			&UToroGameUserSettings::SetViewDistanceQuality,
			EUserSettingImpact::Medium
		);

		AddQuality(
			Graphics_AntiAliasingQuality,
			LOCTEXT("AAQuality", "Anti-Aliasing Quality"),
			LOCTEXT("AAQualityHelp",
				"Adjust the quality of edge smoothing for the selected Image Fidelity method. The effect on appearance and performance depends on that method."),
			Baseline.AntiAliasingQuality,
			&UToroGameUserSettings::GetAntiAliasingQuality,
			&UToroGameUserSettings::SetAntiAliasingQuality,
			EUserSettingImpact::Medium
		);

		AddQuality(
			Graphics_ShadowQuality,
			LOCTEXT("Shadows", "Shadow Quality"),
			LOCTEXT("ShadowsHelp",
				"Adjust the detail and quality of shadows. Higher settings can significantly reduce performance."),
			Baseline.ShadowQuality,
			&UToroGameUserSettings::GetShadowQuality,
			&UToroGameUserSettings::SetShadowQuality,
			EUserSettingImpact::High
		);

		AddQuality(
			Graphics_GlobalIlluminationQuality,
			LOCTEXT("GI", "Global Illumination Quality"),
			LOCTEXT("GIHelp",
				"Adjust the quality of indirect lighting from light bouncing between surfaces. Higher settings can improve lighting accuracy but reduce performance."),
			Baseline.GlobalIlluminationQuality,
			&UToroGameUserSettings::GetGlobalIlluminationQuality,
			&UToroGameUserSettings::SetGlobalIlluminationQuality,
			EUserSettingImpact::High
		);

		AddQuality(
			Graphics_ReflectionQuality,
			LOCTEXT("Reflections", "Reflection Quality"),
			LOCTEXT("ReflectionsHelp",
				"Adjust the quality of reflections on reflective surfaces. Higher settings can improve reflected detail but reduce performance."),
			Baseline.ReflectionQuality,
			&UToroGameUserSettings::GetReflectionQuality,
			&UToroGameUserSettings::SetReflectionQuality,
			EUserSettingImpact::High
		);

		AddQuality(
			Graphics_PostProcessQuality,
			LOCTEXT("PostProcess", "Post-Processing Quality"),
			LOCTEXT("PostProcessHelp",
				"Adjust the quality of image effects such as bloom, depth of field, and motion blur. Higher settings can reduce performance."),
			Baseline.PostProcessQuality,
			&UToroGameUserSettings::GetPostProcessingQuality,
			&UToroGameUserSettings::SetPostProcessingQuality,
			EUserSettingImpact::Medium
		);

		AddQuality(
			Graphics_TextureQuality,
			LOCTEXT("Textures", "Texture Quality"),
			LOCTEXT("TexturesHelp",
				"Adjust the detail of surface textures. Higher settings use more video memory; exceeding available memory can cause blurry textures or stuttering."),
			Baseline.TextureQuality,
			&UToroGameUserSettings::GetTextureQuality,
			&UToroGameUserSettings::SetTextureQuality,
			EUserSettingImpact::Medium
		);

		AddQuality(
			Graphics_EffectsQuality,
			LOCTEXT("Effects", "Effects Quality"),
			LOCTEXT("EffectsHelp",
				"Adjust the quality of visual effects such as particles. Higher settings can make effects more detailed but reduce performance during busy scenes."),
			Baseline.EffectsQuality,
			&UToroGameUserSettings::GetVisualEffectQuality,
			&UToroGameUserSettings::SetVisualEffectQuality,
			EUserSettingImpact::High
		);

		AddQuality(
			Graphics_FoliageQuality,
			LOCTEXT("Foliage", "Foliage Quality"),
			LOCTEXT("FoliageHelp",
				"Adjust the density of grass and foliage affected by this setting. Higher settings make vegetation fuller but can reduce performance."),
			Baseline.FoliageQuality,
			&UToroGameUserSettings::GetFoliageQuality,
			&UToroGameUserSettings::SetFoliageQuality,
			EUserSettingImpact::Medium
		);

		AddQuality(
			Graphics_ShadingQuality,
			LOCTEXT("Shading", "Shading Quality"),
			LOCTEXT("ShadingHelp",
				"Adjust the quality of supported material shading effects, such as how surfaces respond to light. Higher settings can reduce performance."),
			Baseline.ShadingQuality,
			&UToroGameUserSettings::GetShadingQuality,
			&UToroGameUserSettings::SetShadingQuality,
			EUserSettingImpact::Medium
		);

		AddQuality(
			Graphics_LandscapeQuality,
			LOCTEXT("Landscape", "Landscape Quality"),
			LOCTEXT("LandscapeHelp",
				"Adjust terrain detail. Higher settings can preserve more detail at a distance but reduce performance."),
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
		VolumeSection.GetMetadata().Description = LOCTEXT("VolumeSectionDescription",
			"Adjust master volume and individual audio categories.");
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
					[Key](const UToroGameUserSettings& Target, const FUserSettingEntry&)
					{
						return Target.GetAudioVolume(Key);
					},
					[Key](UToroGameUserSettings& Target, const FUserSettingEntry&, const float& Value)
					{
						Target.SetAudioVolume(Key, Value);
					}
				);
		};

		AddVolume(
			Audio_MasterVolume,
			LOCTEXT("MasterVolume", "Master"),
			LOCTEXT("MasterVolumeHelp", "Adjust the overall volume of all audio. Set to 0 to mute the game.")
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
			LOCTEXT("DialogueVolumeHelp", "Adjust the volume of speech and character voices.")
		);

		AddVolume(
			Audio_UIVolume,
			LOCTEXT("UIVolume", "User Interface"),
			LOCTEXT("UIVolumeHelp", "Adjust the volume of menu and interface sounds, such as button clicks.")
		);
	}

	auto& DebugPage = Registry.EditPage(TEXT("Debug"), LOCTEXT("DebugPage", "Debug"));
	DebugPage.GetMetadata().Description = LOCTEXT("DebugPageDescription", "Debugging options for developer.");
	DebugPage.GetMetadata().Impact = EUserSettingImpact::None;
	{
		auto& FlagSection = DebugPage.EditSection(TEXT("Flags"), LOCTEXT("FlagsSection", "Flags"));
		FlagSection.GetMetadata().Description = LOCTEXT("FlagsSectionDescription", "Adjust internal developer flags.");
		FlagSection.GetMetadata().Impact = EUserSettingImpact::None;

		for (const FString& Flag : UserSettingFlags::AllToggleable)
		{
			FText Desc = LOCTEXT("FlagSettingDefaultHelp", "Flag that toggles something... or nothing");
			if (const IConsoleVariable* CVar = UToroCVarLibrary::FindCVar(Flag))
			{
				Desc = FText::FromString(CVar->GetHelp());
			}

			FlagSection.AddSelector<int32>(*Flag)
				.DisplayName(FText::FromString(Flag))
				.Description(MoveTemp(Desc))
				.Default(0)
				.Option(0, LOCTEXT("FlagOff", "Off"))
				.Option(1, LOCTEXT("FlagOn", "On"))
				.BindCallbacks(
					&Settings,
					[Flag](const UToroGameUserSettings&, const FUserSettingEntry&)
					{
						return UToroCVarLibrary::GetCVarInt(Flag);
					},
					[Flag](UToroGameUserSettings&, const FUserSettingEntry&, const int32& Value)
					{
						UToroCVarLibrary::SetCVarInt(Flag, Value != 0);
					}
				);
		}
	}
}
#undef LOCTEXT_NAMESPACE
