// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "GameFramework/WorldSettings.h"
#include "LightProbes/LightProbeSystemConfig.h"
#include "ToroWorldSettings.generated.h"

class UWorldPostProcessComponent;

UCLASS(Blueprintable, BlueprintType, PrioritizeCategories = (Systems))
class TOROSYSTEMS_API AToroWorldSettings : public AWorldSettings
{
	GENERATED_BODY()

public:

	AToroWorldSettings();

	[[nodiscard]] UWorldPostProcessComponent* GetPostProcess() const { return PostProcess; }
	[[nodiscard]] const FLightProbeSystemConfig& GetLightProbeConfig() const { return LightProbes; }

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Subobjects)
	TObjectPtr<UWorldPostProcessComponent> PostProcess;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Systems)
	FLightProbeSystemConfig LightProbes;
};
