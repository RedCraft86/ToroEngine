// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "Libraries/ToroPlatformLibrary.h"
#include "HAL/PlatformApplicationMisc.h"
#include "HAL/PlatformFileManager.h"
#include "HAL/PlatformProperties.h"
#include "Misc/CoreDelegates.h"
#include "Misc/FileHelper.h"
#include "UObject/Stack.h"
#include "IPlatformFilePak.h"

void UToroPlatformLibrary::CopyStringToClipboard(const FString& InString)
{
	FPlatformApplicationMisc::ClipboardCopy(*InString);
}

FString UToroPlatformLibrary::GetStringFromClipboard()
{
	FString OutString;
	FPlatformApplicationMisc::ClipboardPaste(OutString);
	return OutString;
}

bool UToroPlatformLibrary::MountPakFile(const FString& FilePath)
{
	if (!FPlatformProperties::RequiresCookedData())
	{
		FFrame::KismetExecutionMessage(
			TEXT("Current project does not allow cooked content. This should not be called in-editor."),
			ELogVerbosity::Error
		);
		return false;
	}

	if (!FCoreDelegates::MountPak.IsBound())
	{
		FFrame::KismetExecutionMessage(
			TEXT("Pak mounting has not been initialized by the engine."),
			ELogVerbosity::Error
		);
		return false;
	}

	return FCoreDelegates::MountPak.Execute(FilePath, 0) != nullptr;
}

bool UToroPlatformLibrary::UnmountPakFile(const FString& FilePath)
{
	if (!FPlatformProperties::RequiresCookedData())
	{
		FFrame::KismetExecutionMessage(
			TEXT("Current project does not allow cooked content. This should not be called in-editor."),
			ELogVerbosity::Error
		);
		return false;
	}

	if (!FCoreDelegates::OnUnmountPak.IsBound())
	{
		FFrame::KismetExecutionMessage(
			TEXT("Pak mounting has not been initialized by the engine."),
			ELogVerbosity::Error
		);
		return false;
	}

	return FCoreDelegates::OnUnmountPak.Execute(FilePath);
}

void UToroPlatformLibrary::GetMountedPakFileNames(TArray<FString>& FileNames)
{
	FileNames.Reset();

	FPakPlatformFile* PakFileManager = static_cast<FPakPlatformFile*>(
		FPlatformFileManager::Get().GetPlatformFile(TEXT("PakFile"))
	);

	if (PakFileManager && PakFileManager->GetLowerLevel())
	{
		PakFileManager->GetMountedPakFilenames(FileNames);
	}
}

bool UToroPlatformLibrary::ReadFileToString(const FString& FilePath, FString& OutContent)
{
	OutContent.Empty();
	return FFileHelper::LoadFileToString(OutContent, *FilePath);
}

bool UToroPlatformLibrary::WriteStringToFile(const FString& FilePath, const FString& InContent)
{
	return FFileHelper::SaveStringToFile(InContent, *FilePath);
}
