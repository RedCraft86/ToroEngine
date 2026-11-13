// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "UserSettingsTypes.h"
#include "GameplayTagContainer.h"
#include "GameFramework/GameUserSettings.h"
#include "ToroGameUserSettings.generated.h"

class UGameInstance;
class AToroWorldSettings;

USTRUCT()
struct FScalabilityBaseline
{
	GENERATED_BODY()

	UPROPERTY() bool bSaved						= false;
	UPROPERTY() float ResolutionQuality			= 100.0f;
	UPROPERTY() int32 ViewDistanceQuality		= 3;
	UPROPERTY() int32 AntiAliasingQuality		= 3;
	UPROPERTY() int32 ShadowQuality				= 3;
	UPROPERTY() int32 GlobalIlluminationQuality	= 3;
	UPROPERTY() int32 ReflectionQuality			= 3;
	UPROPERTY() int32 PostProcessQuality		= 3;
	UPROPERTY() int32 TextureQuality			= 3;
	UPROPERTY() int32 EffectsQuality			= 3;
	UPROPERTY() int32 FoliageQuality			= 3;
	UPROPERTY() int32 ShadingQuality			= 3;
	UPROPERTY() int32 LandscapeQuality			= 3;

	void Capture(const Scalability::FQualityLevels& Quality);
	void Restore(Scalability::FQualityLevels& Quality) const;
};

UCLASS(Blueprintable, BlueprintType)
class TOROSYSTEMS_API UToroGameUserSettings : public UGameUserSettings
{
	GENERATED_BODY()

public:

	UToroGameUserSettings();

	/**
	 * Retrieves the engine's active game user settings if they match the requested class.
	 * @param Class Optional class filter; a null or invalid class disables type filtering.
	 * @return The found object, or nullptr if unavailable, invalid, or rejected by a valid class filter.
	 */
	UFUNCTION(BlueprintPure, Category = ToroEngine, DisplayName = "Get Game User Settings",
		meta = (DeterminesOutputType = Class))
	[[nodiscard]] static UGameUserSettings* Get(
		UPARAM(meta = (AllowAbstract = false)) TSubclassOf<UGameUserSettings> Class);

	/**
	 * Retrieves the engine's active game user settings as the requested type.
	 * @tparam T Type derived from UGameUserSettings.
	 * @return The matching object cast to T, or nullptr if unavailable, invalid, or of a different type.
	 */
	template<typename T = UToroGameUserSettings>
	[[nodiscard]] static T* Get()
	{
		static_assert(TIsDerivedFrom<T, UGameUserSettings>::Value,
			"T must derive from UGameUserSettings");
		return Cast<T>(Get(T::StaticClass()));
	}

	UFUNCTION(BlueprintCallable, Category = Settings)
	void RevertAllScalabilitySettings();

	UFUNCTION(BlueprintCallable, Category = Settings)
	void SetSensitivityX(float Value);

	UFUNCTION(BlueprintPure, Category = Settings)
	float GetSensitivityX() const;

	UFUNCTION(BlueprintCallable, Category = Settings)
	void SetInvertMouseX(bool Value);

	UFUNCTION(BlueprintPure, Category = Settings)
	bool GetInvertMouseX() const;

	UFUNCTION(BlueprintCallable, Category = Settings)
	void SetSensitivityY(float Value);

	UFUNCTION(BlueprintPure, Category = Settings)
	float GetSensitivityY() const;

	UFUNCTION(BlueprintCallable, Category = Settings)
	void SetInvertMouseY(bool Value);

	UFUNCTION(BlueprintPure, Category = Settings)
	bool GetInvertMouseY() const;

	UFUNCTION(BlueprintCallable, Category = Settings)
	void SetBrightness(float Value);

	UFUNCTION(BlueprintPure, Category = Settings)
	float GetBrightness() const;

	UFUNCTION(BlueprintCallable, Category = Settings)
	void SetMotionBlurAmount(int32 Value);

	UFUNCTION(BlueprintPure, Category = Settings)
	int32 GetMotionBlurAmount() const;

	UFUNCTION(BlueprintCallable, Category = Settings)
	void SetLumenMode(TEnumAsByte<ELumenUserSetting::Type> Value);

	UFUNCTION(BlueprintPure, Category = Settings)
	TEnumAsByte<ELumenUserSetting::Type> GetLumenMode() const;

	UFUNCTION(BlueprintCallable, Category = Settings)
	void SetImageFidelityMode(TEnumAsByte<EImageFidelityMode::Type> Value);

	UFUNCTION(BlueprintPure, Category = Settings)
	TEnumAsByte<EImageFidelityMode::Type> GetImageFidelityMode() const;

	UFUNCTION(BlueprintCallable, Category = Settings)
	void SetTSRScreenPercentage(float Value);

	UFUNCTION(BlueprintPure, Category = Settings)
	float GetTSRScreenPercentage() const;

	UFUNCTION(BlueprintCallable, Category = Settings)
	void SetAudioVolume(UPARAM(meta = (Categories = "UserSettings.Audio")) FGameplayTag Channel, float Value);

	UFUNCTION(BlueprintPure, Category = Settings)
	float GetAudioVolume(UPARAM(meta = (Categories = "UserSettings.Audio")) FGameplayTag Channel) const;

	UFUNCTION(BlueprintCallable, Category = Settings)
	void AddUserFlagEntry(const FString Key);

	UFUNCTION(BlueprintCallable, Category = Settings)
	void SetUserFlagEnabled(const FString Key, bool bEnabled);

	UFUNCTION(BlueprintPure, Category = Settings)
	bool IsUserFlagEnabled(const FString Key) const;

	[[nodiscard]] const FScalabilityBaseline& GetScalabilityBaseline() const
	{
		return ScalabilityBaseline;
	}

	void Initialize(UGameInstance& GameInst);
	void TryApplyResolutionSettings();

	virtual UWorld* GetWorld() const override;

	virtual void SetToDefaults() override;
	virtual void ValidateSettings() override;
	virtual void ApplyNonResolutionSettings() override;
	virtual void RunHardwareBenchmark(int32 WorkScale = 10, float CPUScale = 1.0f, float GPUScale = 1.0f) override;
	virtual void SetOverallScalabilityLevel(int32 Value) override;

	[[nodiscard]] FOnGameUserSettingsUINeedsUpdate& GetOnUIUpdate();
	[[nodiscard]] FSimpleMulticastDelegate::RegistrationType& GetOnSettingsApplied();

protected:

	FSimpleMulticastDelegate OnSettingsAppliedEvent;

	void ApplyBrightness() const;
	void ApplyMotionBlur() const;
	void ApplyLumenSettings() const;
	void ApplyFidelitySettings() const;
	void ApplyAudioVolume(const FGameplayTag& Channel) const;
	void OnResolutionDialogClosed(int32 ButtonIndex);

private:

	bool bIsFirstLoad = false;
	TWeakObjectPtr<UGameInstance> GameInstance;
	TWeakObjectPtr<AToroWorldSettings> WorldSettings;

	UPROPERTY(Config)
	float SensitivityX;

	UPROPERTY(Config)
	bool bInvertMouseX;

	UPROPERTY(Config)
	float SensitivityY;

	UPROPERTY(Config)
	bool bInvertMouseY;

	UPROPERTY(Config)
	float Brightness;

	UPROPERTY(Config)
	int32 MotionBlurAmount;

	UPROPERTY(Config)
	TEnumAsByte<ELumenUserSetting::Type> LumenMode;

	UPROPERTY(Config)
	TEnumAsByte<EImageFidelityMode::Type> ImageFidelityMode;

	UPROPERTY(Config)
	float TSRScreenPercentage;

	UPROPERTY(Config)
	TArray<FUserAudioVolume> AudioVolumes;

	UPROPERTY(Config)
	TArray<FUserDebugFlag> DebugFlags;

	UPROPERTY(Config)
	FScalabilityBaseline ScalabilityBaseline;
};
