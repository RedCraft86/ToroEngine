// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "ToroDevEdSettings.h"
#include "Utilities/ToroLogging.h"
#include "Interfaces/IPluginManager.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Framework/Notifications/NotificationManager.h"
#include "HAL/FileManager.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Editor.h"

#define LOCTEXT_NAMESPACE "ToroEngine"
UToroDevEdSettings::UToroDevEdSettings()
{
	CategoryName = TEXT("Project");
	SectionName = TEXT("ToroEngineEditor");

	ApplyDefaultConfigsButton.OnClicked().AddStatic(&UToroDevEdSettings::ApplyDefaultConfigs);
}

void UToroDevEdSettings::ApplyDefaultConfigs()
{
	const auto ReportResult = [](bool bSuccess, const FText& Message)
	{
		if (bSuccess)
		{
			TORO_LOG_INFO(TEXT("%s"), *Message.ToString());
		}
		else
		{
			TORO_LOG_ERROR(TEXT("%s"), *Message.ToString());
		}

		FNotificationInfo Info(Message);
		Info.ExpireDuration = 10.0f;
		if (const TSharedPtr<SNotificationItem> Item = FSlateNotificationManager::Get().AddNotification(Info))
		{
			Item->SetCompletionState(bSuccess ? SNotificationItem::CS_Success : SNotificationItem::CS_Fail);
		}
	};

	if (GEditor && GEditor->PlayWorld)
	{
		ReportResult(false, LOCTEXT("PIEActive", "End Play or Simulate before applying default configs."));
		return;
	}

	const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(UE_PLUGIN_NAME);
	if (!Plugin.IsValid())
	{
		ReportResult(false, LOCTEXT("ToroEngineMissingPlugin", "Cannot locate the ToroEngine plugin."));
		return;
	}

	const FString PresetPath = FPaths::Combine(Plugin->GetBaseDir(), TEXT("Config/Presets/EngineDefaults.ini"));

	FString PresetText;
	if (!FFileHelper::LoadFileToString(PresetText, *PresetPath))
	{
		ReportResult(false, FText::Format(
			LOCTEXT("EngineConfigPresetReadFailed", "Cannot read the preset: {0}"),
			FText::FromString(PresetPath)
		));
		return;
	}

	FConfigFile Preset;
	Preset.ProcessInputFileContents(PresetText, PresetPath);

	int32 EntryCount = 0;
	for (const auto& Section : Preset)
	{
		EntryCount += Section.Value.Num();
	}

	if (EntryCount == 0)
	{
		ReportResult(false, LOCTEXT("EngineConfigPresetEmpty",
			"The ToroEngine preset contains no active settings."
		));
		return;
	}

	const FString Destination = FPaths::Combine(FPaths::ProjectConfigDir(), TEXT("DefaultEngine.ini"));

	FConfigFile ProjectConfig;
	if (IFileManager::Get().FileExists(*Destination))
	{
		FString ProjectText;
		if (!FFileHelper::LoadFileToString(ProjectText, *Destination))
		{
			ReportResult(false, FText::Format(
				LOCTEXT("EngineConfigReadFailed", "Cannot read project config: {0}"),
				FText::FromString(Destination)
			));
			return;
		}
		ProjectConfig.ProcessInputFileContents(ProjectText, Destination);
	}

	for (const auto& Section : Preset)
	{
		for (const auto& Entry : Section.Value)
		{
			ProjectConfig.SetString(*Section.Key, *Entry.Key.ToString(), *Entry.Value.GetValue());
		}
	}

	FString MergedText;
	ProjectConfig.WriteToString(MergedText, Destination);
	if (!FFileHelper::SaveStringToFile(MergedText, *Destination))
	{
		ReportResult(false, FText::Format(LOCTEXT("EngineConfigWriteFailed",
			"Cannot save {0}. Check file permissions or source-control checkout."),
			FText::FromString(Destination)
		));
		return;
	}

	if (GConfig)
	{
		for (const auto& Section : Preset)
		{
			for (const auto& Entry : Section.Value)
			{
				GConfig->SetString(*Section.Key, *Entry.Key.ToString(), *Entry.Value.GetValue(), GEngineIni);
			}
		}
	}

	ReportResult(true, FText::Format(LOCTEXT("AppliedEngineConfig",
		"Saved {0} ToroEngine defaults to DefaultEngine.ini. Restart Unreal Editor to load the new settings."),
		FText::AsNumber(EntryCount)
	));
}

#undef LOCTEXT_NAMESPACE
