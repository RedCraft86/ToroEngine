// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "Curves/CurveFloat.h"
#include "Curves/CurveVector.h"
#include "Curves/CurveLinearColor.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "InlineCurves.generated.h"

/**
 * A Float curve using inline keys or an assigned external curve asset.
 * Queries prefer the external asset when assigned. Editing helpers modify inline keys only
 * and do nothing while an external asset is assigned.
 */
USTRUCT(BlueprintType)
struct TOROENGINE_API FInlineFloatCurve final
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = InlineCurve)
	FRuntimeFloatCurve Curve;

	FORCEINLINE operator FRuntimeFloatCurve&()
	{
		return Curve;
	}

	FORCEINLINE operator const FRuntimeFloatCurve&() const
	{
		return Curve;
	}

	[[nodiscard]] FORCEINLINE const UCurveFloat* GetCurveAsset() const
	{
		return Curve.ExternalCurve;
	}

	[[nodiscard]] const FRichCurve* GetRichCurve() const;
	[[nodiscard]] FRichCurve* GetRichCurve();

	[[nodiscard]] bool HasAnyData() const;
	[[nodiscard]] float GetValue(float Time, float Default = 0.0f) const;

	void GetTimeRange(float& Min, float& Max) const;
	void GetValueRange(float& Min, float& Max) const;

	void ResetCurve();
	void RemovePoint(float Time);
	void AddOrUpdatePoint(float Time, float Value, ERichCurveTangentMode Tangent = RCTM_Auto);
};

/**
 * A Vector curve using inline keys or an assigned external curve asset.
 * Queries prefer the external asset when assigned. Editing helpers modify inline keys only
 * and do nothing while an external asset is assigned.
 */
USTRUCT(BlueprintType)
struct TOROENGINE_API FInlineVectorCurve final
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = InlineCurve)
	FRuntimeVectorCurve Curve;

	FORCEINLINE operator FRuntimeVectorCurve&()
	{
		return Curve;
	}

	FORCEINLINE operator const FRuntimeVectorCurve&() const
	{
		return Curve;
	}

	[[nodiscard]] FORCEINLINE const UCurveVector* GetCurveAsset() const
	{
		return Curve.ExternalCurve;
	}

	[[nodiscard]] const FRichCurve* GetRichCurve(uint8 Idx) const;
	[[nodiscard]] FRichCurve* GetRichCurve(uint8 Idx);

	[[nodiscard]] bool HasAnyData() const;
	[[nodiscard]] FVector GetValue(float Time, const FVector& Default = FVector::ZeroVector) const;

	void GetTimeRange(float& Min, float& Max) const;
	void GetValueRange(FVector& Min, FVector& Max) const;

	void ResetCurve();
	void RemovePoint(float Time);
	void AddOrUpdatePoint(float Time, const FVector& Value, ERichCurveTangentMode Tangent = RCTM_Auto);
};

/**
 * A Color curve using inline keys or an assigned external curve asset.
 * Queries prefer the external asset when assigned. Editing helpers modify inline keys only
 * and do nothing while an external asset is assigned.
 * Evaluation reads raw RGBA channels without applying asset color-adjustment settings.
 */
USTRUCT(BlueprintType)
struct TOROENGINE_API FInlineColorCurve final
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = InlineCurve)
	FRuntimeCurveLinearColor Curve;

	FORCEINLINE operator FRuntimeCurveLinearColor&()
	{
		return Curve;
	}

	FORCEINLINE operator const FRuntimeCurveLinearColor&() const
	{
		return Curve;
	}

	[[nodiscard]] FORCEINLINE const UCurveLinearColor* GetCurveAsset() const
	{
		return Curve.ExternalCurve;
	}

	[[nodiscard]] const FRichCurve* GetRichCurve(uint8 Idx) const;
	[[nodiscard]] FRichCurve* GetRichCurve(uint8 Idx);

	[[nodiscard]] bool HasAnyData() const;
	[[nodiscard]] FLinearColor GetValue(float Time, const FLinearColor& Default = FLinearColor::Black) const;

	void GetTimeRange(float& Min, float& Max) const;
	void GetValueRange(FLinearColor& Min, FLinearColor& Max) const;

	void ResetCurve();
	void RemovePoint(float Time);
	void AddOrUpdatePoint(float Time, const FLinearColor& Value, ERichCurveTangentMode Tangent = RCTM_Auto);
};

/**
 * Blueprint queries and inline-key editing for float, vector, and color curves.
 * Queries use an external asset when assigned; editing never modifies external assets.
 */
UCLASS(NotBlueprintable, NotBlueprintType)
class TOROENGINE_API UInlineCurveLibrary final : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/** Returns the external curve asset, or null when the curve uses inline data. */
	UFUNCTION(BlueprintPure, Category = "Math|InlineCurves|Float")
	static UCurveFloat* GetFloatCurveAsset(const FInlineFloatCurve& Curve);

	/** Returns whether the active inline or external curve has any data. */
	UFUNCTION(BlueprintPure, Category = "Math|InlineCurves|Float")
	static bool HasFloatCurveData(const FInlineFloatCurve& Curve);

	/**
	 * Evaluates the active inline or external curve at Time.
	 * @param DefaultValue Fallback used by channels without keys or a curve default value.
	 */
	UFUNCTION(BlueprintPure, Category = "Math|InlineCurves|Float")
	static float GetFloatCurveValue(const FInlineFloatCurve& Curve, float Time, float DefaultValue = 0.0f);

	/**
	 * Gets the earliest and latest key times across the active curve channels.
	 * Both outputs are zero when no channel contains keys.
	 */
	UFUNCTION(BlueprintPure, Category = "Math|InlineCurves|Float")
	static void GetFloatCurveTimeRange(const FInlineFloatCurve& Curve, float& Min, float& Max);

	/**
	 * Gets the minimum and maximum values for each active curve channel.
	 * Channels without keys produce zero bounds.
	 */
	UFUNCTION(BlueprintPure, Category = "Math|InlineCurves|Float")
	static void GetFloatCurveValueRange(const FInlineFloatCurve& Curve, float& Min, float& Max);

	/**
	 * Clears inline curve keys.
	 * Does nothing when an external curve asset is assigned.
	 */
	UFUNCTION(BlueprintCallable, Category = "Math|InlineCurves|Float")
	static void ResetFloatCurve(UPARAM(ref) FInlineFloatCurve& Curve);

	/**
	 * Removes keys at Time from the inline curve channels.
	 * Does nothing when an external curve asset is assigned.
	 */
	UFUNCTION(BlueprintCallable, Category = "Math|InlineCurves|Float")
	static void RemoveFloatCurvePoint(UPARAM(ref) FInlineFloatCurve& Curve, float Time);

	/**
	 * Adds or updates inline channel keys at Time and applies the selected tangent mode.
	 * Does nothing when an external curve asset is assigned.
	 */
	UFUNCTION(BlueprintCallable, Category = "Math|InlineCurves|Float")
	static void AddOrUpdateFloatCurvePoint(UPARAM(ref) FInlineFloatCurve& Curve, float Time, float Value, TEnumAsByte<ERichCurveTangentMode> Tangent = RCTM_Auto);

	/** Returns the external curve asset, or null when the curve uses inline data. */
	UFUNCTION(BlueprintPure, Category = "Math|InlineCurves|Vector")
	static UCurveVector* GetVectorCurveAsset(const FInlineVectorCurve& Curve);

	/** Returns whether the active inline or external curve has any data. */
	UFUNCTION(BlueprintPure, Category = "Math|InlineCurves|Vector")
	static bool HasVectorCurveData(const FInlineVectorCurve& Curve);

	/**
	 * Evaluates the active inline or external curve at Time.
	 * @param DefaultValue Fallback used by channels without keys or a curve default value.
	 */
	UFUNCTION(BlueprintPure, Category = "Math|InlineCurves|Vector")
	static FVector GetVectorCurveValue(const FInlineVectorCurve& Curve, float Time, const FVector& DefaultValue = FVector::ZeroVector);

	/**
	 * Gets the earliest and latest key times across the active curve channels.
	 * Both outputs are zero when no channel contains keys.
	 */
	UFUNCTION(BlueprintPure, Category = "Math|InlineCurves|Vector")
	static void GetVectorCurveTimeRange(const FInlineVectorCurve& Curve, float& Min, float& Max);

	/**
	 * Gets the minimum and maximum values for each active curve channel.
	 * Channels without keys produce zero bounds.
	 */
	UFUNCTION(BlueprintPure, Category = "Math|InlineCurves|Vector")
	static void GetVectorCurveValueRange(const FInlineVectorCurve& Curve, FVector& Min, FVector& Max);

	/**
	 * Clears inline curve keys.
	 * Does nothing when an external curve asset is assigned.
	 */
	UFUNCTION(BlueprintCallable, Category = "Math|InlineCurves|Vector")
	static void ResetVectorCurve(UPARAM(ref) FInlineVectorCurve& Curve);

	/**
	 * Removes keys at Time from the inline curve channels.
	 * Does nothing when an external curve asset is assigned.
	 */
	UFUNCTION(BlueprintCallable, Category = "Math|InlineCurves|Vector")
	static void RemoveVectorCurvePoint(UPARAM(ref) FInlineVectorCurve& Curve, float Time);

	/**
	 * Adds or updates inline channel keys at Time and applies the selected tangent mode.
	 * Does nothing when an external curve asset is assigned.
	 */
	UFUNCTION(BlueprintCallable, Category = "Math|InlineCurves|Vector")
	static void AddOrUpdateVectorCurvePoint(UPARAM(ref) FInlineVectorCurve& Curve, float Time, const FVector& Value, TEnumAsByte<ERichCurveTangentMode> Tangent = RCTM_Auto);

	/** Returns the external curve asset, or null when the curve uses inline data. */
	UFUNCTION(BlueprintPure, Category = "Math|InlineCurves|Color")
	static UCurveLinearColor* GetColorCurveAsset(const FInlineColorCurve& Curve);

	/** Returns whether the active inline or external curve has any data. */
	UFUNCTION(BlueprintPure, Category = "Math|InlineCurves|Color")
	static bool HasColorCurveData(const FInlineColorCurve& Curve);

	/**
	 * Evaluates the active inline or external curve at Time.
	 * @param DefaultValue Fallback used by channels without keys or a curve default value.
	 */
	UFUNCTION(BlueprintPure, Category = "Math|InlineCurves|Color")
	static FLinearColor GetColorCurveValue(const FInlineColorCurve& Curve, float Time, const FLinearColor& DefaultValue = FLinearColor::Black);

	/**
	 * Gets the earliest and latest key times across the active curve channels.
	 * Both outputs are zero when no channel contains keys.
	 */
	UFUNCTION(BlueprintPure, Category = "Math|InlineCurves|Color")
	static void GetColorCurveTimeRange(const FInlineColorCurve& Curve, float& Min, float& Max);

	/**
	 * Gets the minimum and maximum values for each active curve channel.
	 * Channels without keys produce zero bounds.
	 */
	UFUNCTION(BlueprintPure, Category = "Math|InlineCurves|Color")
	static void GetColorCurveValueRange(const FInlineColorCurve& Curve, FLinearColor& Min, FLinearColor& Max);

	/**
	 * Clears inline curve keys.
	 * Does nothing when an external curve asset is assigned.
	 */
	UFUNCTION(BlueprintCallable, Category = "Math|InlineCurves|Color")
	static void ResetColorCurve(UPARAM(ref) FInlineColorCurve& Curve);

	/**
	 * Removes keys at Time from the inline curve channels.
	 * Does nothing when an external curve asset is assigned.
	 */
	UFUNCTION(BlueprintCallable, Category = "Math|InlineCurves|Color")
	static void RemoveColorCurvePoint(UPARAM(ref) FInlineColorCurve& Curve, float Time);

	/**
	 * Adds or updates inline channel keys at Time and applies the selected tangent mode.
	 * Does nothing when an external curve asset is assigned.
	 */
	UFUNCTION(BlueprintCallable, Category = "Math|InlineCurves|Color")
	static void AddOrUpdateColorCurvePoint(UPARAM(ref) FInlineColorCurve& Curve, float Time, const FLinearColor& Value, TEnumAsByte<ERichCurveTangentMode> Tangent = RCTM_Auto);
};
