// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "UE5Coro.h"
#include "UserInterface/Bases/ToroMenuWidget.h"
#include "ToroPauseWidget.generated.h"

class UTextBlock;
class UCommonButtonBase;

UCLASS(MinimalAPI, Abstract, Blueprintable, BlueprintType)
class UToroPauseWidget final : public UToroMenuWidget
{
	GENERATED_BODY()

public:

	UToroPauseWidget();

	UFUNCTION(BlueprintCallable, Category = UserInterface,
		DisplayName = "Push Pause", meta = (WorldContext = ContextObject))
	static TOROSYSTEMS_API bool Push(const UObject* ContextObject, int32 PlayerIdx = 0);

private:

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> ResumeButton;

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> SettingsButton;

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> ReloadButton;

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> MenuButton;

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UTextBlock> VersionBlock;

	void OnResumeGame();
	void OnSettingsMenu() const;
	void OnReloadGame();
	void OnMainMenu();

	void OnProcessReload(int32 Option);
	void OnProcessMainMenu(int32 Option);
	UE5Coro::TCoroutine<> ProcessMapChange(const TSoftObjectPtr<UWorld>& TargetMap);

	virtual void NativeConstruct() override;
};
