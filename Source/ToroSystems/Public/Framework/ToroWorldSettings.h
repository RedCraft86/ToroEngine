// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "GameFramework/WorldSettings.h"
#include "LightProbes/LightProbeSystemConfig.h"
#include "WorldMusic/WorldMusicSystemConfig.h"
#include "ToroWorldSettings.generated.h"

class UWorldPostProcessComponent;
class UToroGameInstance;

UCLASS(Blueprintable, BlueprintType, PrioritizeCategories = (Systems))
class TOROSYSTEMS_API AToroWorldSettings : public AWorldSettings
{
	GENERATED_BODY()

public:

	AToroWorldSettings();

	/**
	 * Retrieves the active world settings if it matches the requested class.
	 * @param Class Optional class filter; a null or invalid class disables type filtering.
	 * @return The found object, or nullptr if unavailable, invalid, or rejected by a valid class filter.
	 */
	UFUNCTION(BlueprintPure, Category = ToroEngine, DisplayName = "Get World Settings",
		meta = (WorldContext = ContextObject, DeterminesOutputType = Class))
	[[nodiscard]] static AWorldSettings* Get(const UObject* ContextObject,
		UPARAM(meta = (AllowAbstract = false)) TSubclassOf<AWorldSettings> Class);

	/**
	 * Retrieves the active world settings as the requested type.
	 * @tparam T Type derived from AWorldSettings.
	 * @return The matching object cast to T, or nullptr if unavailable, invalid, or of a different type.
	 */
	template<typename T = AToroWorldSettings>
	[[nodiscard]] static T* Get(const UObject* ContextObject)
	{
		static_assert(TIsDerivedFrom<T, AWorldSettings>::Value,
			"T must derive from AWorldSettings");
		return Cast<T>(Get(ContextObject, T::StaticClass()));
	}

	UFUNCTION(BlueprintPure, Category = WorldSettings)
	[[nodiscard]] FORCEINLINE bool IsMainLevelSettings() const { return bIsMainLevelSettings; }

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = Audio)
	void SetSoundClassVolume(USoundClass* InClass, float InVolume) const;

	[[nodiscard]] UWorldPostProcessComponent* GetPostProcess() const { return PostProcess; }
	[[nodiscard]] const FLightProbeSystemConfig& GetLightProbeConfig() const { return LightProbes; }
	[[nodiscard]] const FWorldMusicSystemConfig& GetWorldMusicConfig() const { return WorldMusic; }

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Subobjects)
	TObjectPtr<UWorldPostProcessComponent> PostProcess;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Systems)
	FLightProbeSystemConfig LightProbes;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Systems)
	FWorldMusicSystemConfig WorldMusic;

	bool bIsMainLevelSettings;
	TWeakObjectPtr<UToroGameInstance> GameInstance;

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

#if WITH_EDITOR
	virtual void OnConstruction(const FTransform& Transform) override;
#endif
};
