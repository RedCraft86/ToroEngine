// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "SaveGameContainer.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SaveGameSubsystem.generated.h"

UCLASS(MinimalAPI, NotBlueprintable, BlueprintType)
class USaveGameSubsystem final : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:

	USaveGameSubsystem();

	TOROSYSTEMS_API static USaveGameSubsystem* Get(const UObject* Context);

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = SaveSystem)
	TOROSYSTEMS_API bool LoadGlobal() const;

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = SaveSystem)
	TOROSYSTEMS_API bool SaveGlobal() const;

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = SaveSystem)
	TOROSYSTEMS_API bool LoadSlotted(uint8 SlotNum) const;

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = SaveSystem)
	TOROSYSTEMS_API bool SaveSlotted(uint8 SlotNum) const;
	TOROSYSTEMS_API bool SaveSlotted(uint8 SlotNum, TFunctionRef<void(FJsonObject&)> ApplyMetadata) const;

	FGlobalSaveContainer& GetGlobal() const { return *GlobalContainer.Get(); }
	FSlottedSaveContainer& GetSlotted() const { return *SlottedContainer.Get(); }

	FSimpleMulticastDelegate OnLoadedDifferentSlot;

private:

	TUniquePtr<FGlobalSaveContainer> GlobalContainer;
	TUniquePtr<FSlottedSaveContainer> SlottedContainer;

	virtual void Deinitialize() override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
};
