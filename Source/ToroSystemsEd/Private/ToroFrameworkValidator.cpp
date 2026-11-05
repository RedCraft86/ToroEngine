// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "ToroFrameworkValidator.h"
#include "GameFramework/SpectatorPawn.h"
#include "UserSettings/ToroGameUserSettings.h"
#include "Framework/ToroPlayerCameraManager.h"
#include "Framework/ToroGameViewportClient.h"
#include "Framework/ToroPlayerController.h"
#include "Framework/ToroPlayerCharacter.h"
#include "Framework/ToroWorldSettings.h"
#include "Framework/ToroGameInstance.h"
#include "Framework/ToroPlayerState.h"
#include "Framework/ToroGameState.h"
#include "Framework/ToroPlayerHUD.h"
#include "Framework/ToroGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Containers/Ticker.h"

#define LOCTEXT_NAMESPACE "ToroEngine"
bool UToroFrameworkValidator::DoValidation() const
{
	const UWorld* World = GetWorld();
	if (!IsValid(World) || World->bIsTearingDown)
	{
		return false;
	}

	FMessageLog Log(TEXT("PIE"));
	Log.NewPage(FText::FromString(GetWorld()->GetName()));

	auto Validate = [&Log](const UObject* Inst, const UClass* Base, bool bActor, const FText& Role, const FText& Fix)
	{
		if (!IsValid(Inst) || !IsValid(Base) || Inst->IsA(Base))
		{
			return;
		}

		Log.Error(FText::Format(
			LOCTEXT("FrameworkMismatch", "ToroEngine: {4} uses {0}{2}, which must derive from {0}{3}.\n{1}{5}"),
			bActor ? INVTEXT("A") : INVTEXT("U"),
			INVTEXT("            "), // Spaces "ToroEngine: "
			FText::FromString(Inst->GetClass()->GetName()),
			FText::FromString(Base->GetName()),
			Role,
			Fix
		));
	};

	const UGameInstance* GameInstance = World->GetGameInstance();
	Validate(GameInstance, UToroGameInstance::StaticClass(), false,
		LOCTEXT("GameInstanceRole", "Game Instance"),
		LOCTEXT("GameInstanceFix", "Set class in Project Settings > Maps & Modes."));

	if (GameInstance)
	{
		Validate(GameInstance->GetGameViewportClient(), UToroGameViewportClient::StaticClass(), false,
			LOCTEXT("GameViewportClientRole", "Game Viewport Client"),
			LOCTEXT("GameViewportClientFix", "Set class in Project Settings > Engine > General Settings."));
	}

	if (GEngine)
	{
		Validate(GEngine->GameUserSettings, UToroGameUserSettings::StaticClass(), false,
			LOCTEXT("GameUserSettingsRole", "Game User Settings"),
			LOCTEXT("GameUserSettingsFix", "Set class in Project Settings > Engine > General Settings."));
	}

	Validate(World->GetWorldSettings(), AToroWorldSettings::StaticClass(), true,
		LOCTEXT("WorldSettingsRole", "World Settings"),
		LOCTEXT("WorldSettingsFix", "Set class in Project Settings > Engine > General Settings."));

	Validate(World->GetAuthGameMode(), AToroGameMode::StaticClass(), true,
		LOCTEXT("GameModeRole", "Game Mode"),
		LOCTEXT("GameModeFix", "Set class in Project Settings > Maps & Modes."));

	Validate(World->GetGameState(), AToroGameState::StaticClass(), true,
		LOCTEXT("GameStateRole", "Game State"),
		LOCTEXT("GameStateFix", "Set class in active Game Mode class."));

	const APlayerController* Controller = UGameplayStatics::GetPlayerController(this, 0);
	Validate(Controller, AToroPlayerController::StaticClass(), true,
		LOCTEXT("PlayerControllerRole", "Player Controller"),
		LOCTEXT("PlayerControllerFix", "Set class in active Game Mode class."));

	if (Controller)
	{
		Validate(Controller->GetPlayerState<APlayerState>(), AToroPlayerState::StaticClass(), true,
			LOCTEXT("PlayerStateRole", "Player State"),
			LOCTEXT("PlayerStateFix", "Set class in active Game Mode class."));

		Validate(Controller->GetHUD(), AToroPlayerHUD::StaticClass(), true,
			LOCTEXT("PlayerHUDRole", "Player HUD"),
			LOCTEXT("PlayerHUDFix", "Set class in active Game Mode class."));

		Validate(Controller->PlayerCameraManager, AToroPlayerCameraManager::StaticClass(), true,
			LOCTEXT("PlayerCameraManagerRole", "Player Camera Manager"),
			LOCTEXT("PlayerCameraManagerFix", "Set class in active Game Mode class."));

		const APawn* PlayerPawn = Controller->GetPawn();
		if (IsValid(PlayerPawn) && !PlayerPawn->IsA<ASpectatorPawn>())
		{
			Validate(PlayerPawn, AToroPlayerCharacter::StaticClass(), true,
				LOCTEXT("PlayerCharacterRole", "Player Character"),
				LOCTEXT("PlayerCharacterFix", "Set class in active Game Mode class."));
		}
	}

	if (Log.NumMessages() > 0)
	{
		Log.Error(FText::Format(
			LOCTEXT("FrameworkInvalidClosing", "Framework validation failed with {0} errors."),
			FText::AsNumber(Log.NumMessages())
		));
		return true;
	}

	return false;
}

bool UToroFrameworkValidator::DoesSupportWorldType(EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::PIE;
}

void UToroFrameworkValidator::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
	ExecuteOnGameThread(TEXT("ToroFrameworkValidator"),
		[WeakThis = TWeakObjectPtr(this), WeakWorld = TWeakObjectPtr(&InWorld)]()
		{
			if (WeakThis.IsValid() && WeakWorld.IsValid() && !WeakWorld->bIsTearingDown)
			{
				if (WeakThis->DoValidation())
				{
					GEditor->RequestEndPlayMap();
				}
			}
		}
	);
}
#undef LOCTEXT_NAMESPACE
