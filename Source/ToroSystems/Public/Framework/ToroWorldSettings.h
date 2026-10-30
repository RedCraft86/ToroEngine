// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "GameFramework/WorldSettings.h"
#include "ToroWorldSettings.generated.h"

class UWorldPostProcessComponent;

UCLASS(Blueprintable, BlueprintType)
class TOROSYSTEMS_API AToroWorldSettings : public AWorldSettings
{
	GENERATED_BODY()

public:

	AToroWorldSettings();

	[[nodiscard]] FORCEINLINE UWorldPostProcessComponent* GetPostProcess() const
	{
		return PostProcess;
	}

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Subobjects)
	TObjectPtr<UWorldPostProcessComponent> PostProcess;
};
