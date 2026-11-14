// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "UserSettings/UserSettingsTypes.h"
#include "Libraries/ToroCVarLibrary.h"

namespace UserSettingsTags
{
	UE_DEFINE_GAMEPLAY_TAG(Base, "UserSettings");

	// Camera
	UE_DEFINE_GAMEPLAY_TAG(Camera, "UserSettings.Camera");
	UE_DEFINE_GAMEPLAY_TAG(Camera_SensitivityX, "UserSettings.Camera.SensitivityX");
	UE_DEFINE_GAMEPLAY_TAG(Camera_SensitivityY, "UserSettings.Camera.SensitivityY");
	UE_DEFINE_GAMEPLAY_TAG(Camera_InvertMouseX, "UserSettings.Camera.InvertMouseX");
	UE_DEFINE_GAMEPLAY_TAG(Camera_InvertMouseY, "UserSettings.Camera.InvertMouseY");
	UE_DEFINE_GAMEPLAY_TAG(Camera_Smoothing, "UserSettings.Camera.Smoothing");

	// Display
	UE_DEFINE_GAMEPLAY_TAG(Display, "UserSettings.Display");
	UE_DEFINE_GAMEPLAY_TAG(Display_Resolution, "UserSettings.Display.Resolution");
	UE_DEFINE_GAMEPLAY_TAG(Display_Brightness, "UserSettings.Display.Brightness");
	UE_DEFINE_GAMEPLAY_TAG(Display_VSync, "UserSettings.Display.VSync");
	UE_DEFINE_GAMEPLAY_TAG(Display_FrameRateLimit, "UserSettings.Display.FrameRateLimit");
	UE_DEFINE_GAMEPLAY_TAG(Display_ShowFPS, "UserSettings.Display.ShowFPS");

	// Graphics
	UE_DEFINE_GAMEPLAY_TAG(Graphics, "UserSettings.Graphics");
	UE_DEFINE_GAMEPLAY_TAG(Graphics_ImageFidelityMode, "UserSettings.Graphics.ImageFidelityMode");
	UE_DEFINE_GAMEPLAY_TAG(Graphics_ResolutionScale, "UserSettings.Graphics.ResolutionScale");
	UE_DEFINE_GAMEPLAY_TAG(Graphics_TSRScreenPercentage, "UserSettings.Graphics.TSRScreenPercentage");
	UE_DEFINE_GAMEPLAY_TAG(Graphics_LumenMode, "UserSettings.Graphics.LumenMode");
	UE_DEFINE_GAMEPLAY_TAG(Graphics_MotionBlurQuality, "UserSettings.Graphics.MotionBlurQuality");
	UE_DEFINE_GAMEPLAY_TAG(Graphics_AutoDetect, "UserSettings.Graphics.AutoDetect");
	UE_DEFINE_GAMEPLAY_TAG(Graphics_OverallQuality, "UserSettings.Graphics.OverallQuality");
	UE_DEFINE_GAMEPLAY_TAG(Graphics_ViewDistanceQuality, "UserSettings.Graphics.ViewDistanceQuality");
	UE_DEFINE_GAMEPLAY_TAG(Graphics_AntiAliasingQuality, "UserSettings.Graphics.AntiAliasingQuality");
	UE_DEFINE_GAMEPLAY_TAG(Graphics_ShadowQuality, "UserSettings.Graphics.ShadowQuality");
	UE_DEFINE_GAMEPLAY_TAG(Graphics_GlobalIlluminationQuality, "UserSettings.Graphics.GlobalIlluminationQuality");
	UE_DEFINE_GAMEPLAY_TAG(Graphics_ReflectionQuality, "UserSettings.Graphics.ReflectionQuality");
	UE_DEFINE_GAMEPLAY_TAG(Graphics_PostProcessQuality, "UserSettings.Graphics.PostProcessQuality");
	UE_DEFINE_GAMEPLAY_TAG(Graphics_TextureQuality, "UserSettings.Graphics.TextureQuality");
	UE_DEFINE_GAMEPLAY_TAG(Graphics_EffectsQuality, "UserSettings.Graphics.EffectsQuality");
	UE_DEFINE_GAMEPLAY_TAG(Graphics_FoliageQuality, "UserSettings.Graphics.FoliageQuality");
	UE_DEFINE_GAMEPLAY_TAG(Graphics_ShadingQuality, "UserSettings.Graphics.ShadingQuality");
	UE_DEFINE_GAMEPLAY_TAG(Graphics_LandscapeQuality, "UserSettings.Graphics.LandscapeQuality");

	// Audio
	UE_DEFINE_GAMEPLAY_TAG(Audio, "UserSettings.Audio");
	UE_DEFINE_GAMEPLAY_TAG(Audio_MasterVolume, "UserSettings.Audio.MasterVolume");
	UE_DEFINE_GAMEPLAY_TAG(Audio_UIVolume, "UserSettings.Audio.UIVolume");
	UE_DEFINE_GAMEPLAY_TAG(Audio_MusicVolume, "UserSettings.Audio.MusicVolume");
	UE_DEFINE_GAMEPLAY_TAG(Audio_EffectsVolume, "UserSettings.Audio.EffectsVolume");
	UE_DEFINE_GAMEPLAY_TAG(Audio_DialogueVolume, "UserSettings.Audio.DialogueVolume");
}

FUserDebugFlag::FUserDebugFlag(const FString& InKey)
{
	SanitizeString(InKey, Key);
	if (const IConsoleVariable* CVar = UToroCVarLibrary::FindCVar(Key))
	{
		bEnabled = CVar->GetBool();
	}
}

FUserDebugFlag::FUserDebugFlag(const FString& InKey, bool bInEnabled)
	: bEnabled(bInEnabled)
{
	SanitizeString(InKey, Key);
}

bool FUserDebugFlag::IsValid() const
{
	return UToroCVarLibrary::FindCVar(Key) != nullptr;
}

void FUserDebugFlag::ApplyCVar() const
{
	if (IConsoleVariable* CVar = UToroCVarLibrary::FindCVar(Key))
	{
		CVar->Set(bEnabled);
	}
}

void FUserDebugFlag::SanitizeString(const FString& InString, FString& OutString)
{
	OutString.Empty();
	OutString.Reserve(InString.Len());
	for (const TCHAR Character : InString)
	{
		if (!FChar::IsWhitespace(Character))
		{
			OutString.AppendChar(Character);
		}
	}
}
