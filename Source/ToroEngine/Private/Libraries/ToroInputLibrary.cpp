// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "Libraries/ToroInputLibrary.h"
#include "EnhancedInputSubsystems.h"
#include "Kismet/GameplayStatics.h"

void UToroInputLibrary::ResetInputConfig(FToroInputConfig& Config)
{
	Config.Reset();
}

void UToroInputLibrary::SetPlayerInputConfig(const UObject* ContextObject, FToroInputConfig& Config, int32 PlayerIdx)
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(ContextObject, PlayerIdx);
	if (IsValid(PC))
	{
		PC->SetShowMouseCursor(Config.ShouldShowMouse());
		PC->SetInputMode(Config.GetInputMode());
		PC->FlushPressedKeys();
	}
}

UEnhancedInputLocalPlayerSubsystem* UToroInputLibrary::GetEnhancedInputPlayerSubsystem(const UObject* ContextObject, int32 PlayerIdx)
{
	const APlayerController* PC = UGameplayStatics::GetPlayerController(ContextObject, PlayerIdx);
	const ULocalPlayer* LP = IsValid(PC) ? PC->GetLocalPlayer() : nullptr;
	return IsValid(LP) ? LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>() : nullptr;
}

void UToroInputLibrary::AddInputMappingContext(const UObject* ContextObject, const UInputMappingContext* Mappings, int32 Priority, int32 PlayerIdx)
{
	UEnhancedInputLocalPlayerSubsystem* Subsystem = GetEnhancedInputPlayerSubsystem(ContextObject, PlayerIdx);
	if (IsValid(Subsystem))
	{
		Subsystem->AddMappingContext(Mappings, Priority);
	}
}

void UToroInputLibrary::RemoveInputMappingContext(const UObject* ContextObject, const UInputMappingContext* Mappings, int32 PlayerIdx)
{
	UEnhancedInputLocalPlayerSubsystem* Subsystem = GetEnhancedInputPlayerSubsystem(ContextObject, PlayerIdx);
	if (IsValid(Subsystem))
	{
		Subsystem->RemoveMappingContext(Mappings);
	}
}

void UToroInputLibrary::ClearInputMappingContexts(const UObject* ContextObject, int32 PlayerIdx)
{
	UEnhancedInputLocalPlayerSubsystem* Subsystem = GetEnhancedInputPlayerSubsystem(ContextObject, PlayerIdx);
	if (IsValid(Subsystem))
	{
		Subsystem->ClearAllMappings();
	}
}
