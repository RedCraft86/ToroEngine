// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "UserInterface/Widgets/ToroPauseWidget.h"
#include "UserInterface/Widgets/ToroDialogWidget.h"
#include "UserInterface/UserInterfaceConfig.h"
#include "UserSettings/Widgets/ToroSettingsWidget.h"
#include "Libraries/ToroCameraLibrary.h"
#include "Libraries/ToroWorldLibrary.h"
#include "Framework/ToroPlayerHUD.h"
#include "Kismet/GameplayStatics.h"
#include "Utilities/ToroUtilities.h"
#include "Utilities/ToroLogging.h"
#include "CommonButtonBase.h"
#include "ToroDevSettings.h"

UToroPauseWidget::UToroPauseWidget()
{
	bPausesGame = true;
}

bool UToroPauseWidget::Push(const UObject* ContextObject, int32 PlayerIdx)
{
	const AToroPlayerHUD* HUD = AToroPlayerHUD::Get(ContextObject, PlayerIdx);
	if (!IsValid(HUD) || !HUD->HasMasterWidget())
	{
		TORO_REPORT_ERROR(TEXT("Failed to push Pause Widget: HUD or MasterWidget is unavailable."));
		return false;
	}

	const FUserInterfaceConfig* Config = FUserInterfaceConfig::Get();
	const TSubclassOf<UToroPauseWidget> Class = Config ? Config->PauseWidgetClass.LoadSynchronous() : nullptr;
	if (!IsValid(Class) || Class->HasAnyClassFlags(CLASS_Abstract))
	{
		TORO_REPORT_ERROR(TEXT("Failed to push Pause Widget: Failed to obtain config or got an invalid class."));
		return false;
	}

	if (HUD->AddWidgetByClass(EToroMasterWidgetType::Menu, Class))
	{
		return true;
	}

	TORO_REPORT_ERROR(TEXT("Failed to push Pause Widget: Could not create or add widget."));
	return false;
}

void UToroPauseWidget::OnResumeGame()
{
	DeactivateWidget();
}

void UToroPauseWidget::OnSettingsMenu() const
{
	UToroSettingsWidget::Push(GetOwningPlayer(), 0);
}

void UToroPauseWidget::OnReloadGame()
{
	UToroDialogWidget::Create(
		GetOwningPlayer(),
		INVTEXT("Are you sure?"),
		INVTEXT("Proceeding may cause loss of save progress."),
		TArray{INVTEXT("Proceed"), INVTEXT("Cancel")},
		true
	)->BindUObject(this, &UToroPauseWidget::OnProcessReload);
}

void UToroPauseWidget::OnMainMenu()
{
	UToroDialogWidget::Create(
		GetOwningPlayer(),
		INVTEXT("Are you sure?"),
		INVTEXT("Proceeding may cause loss of save progress."),
		TArray{INVTEXT("Proceed"), INVTEXT("Cancel")},
		true
	)->BindUObject(this, &UToroPauseWidget::OnProcessMainMenu);
}

void UToroPauseWidget::OnProcessReload(int32 Option)
{
	if (Option == 0)
	{
		ProcessMapChange(nullptr);
	}
}

void UToroPauseWidget::OnProcessMainMenu(int32 Option)
{
	if (Option == 0)
	{
		if (const UToroDevSettings* Settings = GetDefault<UToroDevSettings>())
		{
			for (const TPair<TSoftObjectPtr<UWorld>, FToroLevelInfo>& Level : Settings->Levels)
			{
				if (Level.Value.LevelType == LevelTypeTags::MainMenu)
				{
					ProcessMapChange(Level.Key);
					return;
				}
			}
		}

		ProcessMapChange(nullptr);
	}
}

UE5Coro::TCoroutine<> UToroPauseWidget::ProcessMapChange(const TSoftObjectPtr<UWorld> TargetMap)
{
	SetVisibility(ESlateVisibility::HitTestInvisible);

	bool bSuccess;
	co_await UToroCameraLibrary::StartCameraFade(ToroEngine::LatentInfo(this), GetOwningPlayer(), bSuccess);

	DeactivateWidget();
	const APlayerController* PC = GetOwningPlayer();
	if (TargetMap.IsNull())
	{
		UToroWorldLibrary::ReloadLevel(PC);
	}
	else
	{
		UGameplayStatics::OpenLevelBySoftObjectPtr(PC, TargetMap);
	}
}

void UToroPauseWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (!IsValid(ResumeButton)
		|| !IsValid(SettingsButton)
		|| !IsValid(ReloadButton)
		|| !IsValid(MenuButton)
		|| !IsValid(VersionBlock))
	{
		RemoveFromParent();
		return;
	}

	ResumeButton->OnClicked().RemoveAll(this);
	ResumeButton->OnClicked().AddUObject(this, &UToroPauseWidget::OnResumeGame);

	SettingsButton->OnClicked().RemoveAll(this);
	SettingsButton->OnClicked().AddUObject(this, &UToroPauseWidget::OnSettingsMenu);

	ReloadButton->OnClicked().RemoveAll(this);
	ReloadButton->OnClicked().AddUObject(this, &UToroPauseWidget::OnReloadGame);

	MenuButton->OnClicked().RemoveAll(this);
	MenuButton->OnClicked().AddUObject(this, &UToroPauseWidget::OnMainMenu);

	if (const UToroDevSettings* Settings = UToroDevSettings::Get())
	{
		VersionBlock->SetText(FText::FromString(Settings->GetVersionInfo()));
	}
}
