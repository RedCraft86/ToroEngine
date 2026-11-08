// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "Framework/ToroGameInstance.h"
#include "UserSettings/ToroGameUserSettings.h"
#include "Kismet/GameplayStatics.h"
#if !WITH_EDITOR
#include "HAL/FileManager.h"
#include "HAL/PlatformProcess.h"
#include "HAL/PlatformApplicationMisc.h"
#include "Serialization/Archive.h"
#include "ShaderPipelineCache.h"
#include "Misc/MessageDialog.h"
#include "Misc/CommandLine.h"
#include "Misc/App.h"
#else
#include "Editor.h"
#endif

namespace
{
	const TCHAR* InstanceLockPath()
	{
		static FString Path = FPaths::ProjectSavedDir() / TEXT("INSTANCE.lock");
		return *Path;
	}
}

UGameInstance* UToroGameInstance::Get(const UObject* ContextObject, TSubclassOf<UGameInstance> Class)
{
	UGameInstance* GI = UGameplayStatics::GetGameInstance(ContextObject);
	return IsValid(GI) && (!IsValid(Class) || GI->IsA(Class)) ? GI : nullptr;
}

void UToroGameInstance::RestartGame()
{
#if WITH_EDITOR
	if (GEditor && GIsPlayInEditorWorld)
	{
		GEditor->RequestEndPlayMap();
	}
	return;
#else
	ClearInstanceLock();
	FPlatformApplicationMisc::RequestMinimize();

	FProcHandle NewProc = FPlatformProcess::CreateProc(
		FPlatformProcess::ExecutablePath(),
		FCommandLine::Get(),
		true,   // Detached
		false,  // Hidden
		false,  // Really hidden
		nullptr,
		0,
		nullptr,
		nullptr
	);

	if (!NewProc.IsValid())
	{
		FMessageDialog::Open(
			EAppMsgCategory::Error,
			EAppMsgType::Ok,
			INVTEXT("Game failed to automatically restart, please manually relaunch the executable."),
			FText::Format(INVTEXT("Cannot restart {0}"), FText::FromString(FApp::GetProjectName()))
		);
	}
	else
	{
		FPlatformProcess::CloseProc(NewProc);
	}

    FPlatformMisc::RequestExit(false);
#endif
}

void UToroGameInstance::WorldBeginPlay()
{
	ReceiveWorldBeginPlay();
}

void UToroGameInstance::WorldEndPlay()
{
	ReceiveWorldEndPlay();
}

void UToroGameInstance::WorldTick(float DeltaTime, bool bGamePaused)
{
	ReceiveWorldTick(DeltaTime, bGamePaused);
}

void UToroGameInstance::Init()
{
#if !WITH_EDITOR
	FShaderPipelineCache::PauseBatching();
	if (!CreateInstanceLock())
	{
		FPlatformApplicationMisc::RequestMinimize();

		FMessageDialog::Open(
			EAppMsgCategory::Warning,
			EAppMsgType::Ok,
			INVTEXT(
				"Another instance of the game is already running, close that instance first to run. "
				"Running multiple instances at the same time will lead to issues such as save corruption."
			),
			FText::Format(INVTEXT("Cannot start {0}"), FText::FromString(FApp::GetProjectName()))
		);

		// Force exit since we do not need configs being flushed or destructors being called
		FPlatformMisc::RequestExitWithStatus(true, 0);
		return;
	}
	FShaderPipelineCache::ResumeBatching();
#endif

	Super::Init();

	if (UToroGameUserSettings* GUS = UToroGameUserSettings::Get())
	{
		// TODO: GUS->Initialize();
	}
}

void UToroGameInstance::Shutdown()
{
#if !WITH_EDITOR
	ClearInstanceLock();
#endif
	Super::Shutdown();
}

#if !WITH_EDITOR
bool UToroGameInstance::CreateInstanceLock()
{
	InstanceLock = IFileManager::Get().CreateFileWriter(InstanceLockPath(), 0);
	return InstanceLock != nullptr;
}

void UToroGameInstance::ClearInstanceLock()
{
	if (InstanceLock)
	{
		InstanceLock->Close();
		delete InstanceLock;
		InstanceLock = nullptr;

		IFileManager::Get().Delete(InstanceLockPath(), false, false, true);
	}
}
#endif