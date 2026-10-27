// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "Components/BillboardComponent.h"
#include "TintedBillboardComponent.generated.h"

/**
 * Camera-facing billboard with a multiplicative linear-color tint, defaulting to white.
 * The tint is applied after view-dependent highlighting and actor coloration, including its alpha.
 * Rendering uses masked blending and the inherited opacity threshold rather than translucent blending.
 */
UCLASS(ClassGroup = (Rendering), meta = (BlueprintSpawnableComponent, ChildCanTick = true))
class TOROENGINE_API UTintedBillboardComponent : public UBillboardComponent
{
	GENERATED_BODY()

public:

	UTintedBillboardComponent();

	/**
	 * Updates the stored tint and recreates the render state when the new value differs.
	 * Values equal within FLinearColor::Equals()'s default tolerance leave the existing tint unchanged.
	 * @param InNewColor Multiplicative linear RGBA tint; values are stored without clamping.
	 */
	UFUNCTION(BlueprintCallable, Category = "Rendering|Components|Sprite")
	virtual void SetSpriteColor(const FLinearColor InNewColor);

	/**
	 * Gets the stored tint before view-dependent display coloring is applied.
	 * The returned reference belongs to this component, must not outlive it, and reflects later tint changes.
	 */
	UFUNCTION(BlueprintPure, Category = "Rendering|Components|Sprite")
	[[nodiscard]] const FLinearColor& GetSpriteColor() const
	{
		return SpriteColor;
	}

protected:

	/** Linear RGBA multiplier for the computed sprite color; white preserves its untinted appearance. */
	UPROPERTY(EditAnywhere, Category = Sprite)
	FLinearColor SpriteColor;

	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;
};
