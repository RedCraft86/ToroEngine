// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

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
struct TOROCORE_API FInlineFloatCurve final
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = InlineCurve)
	FRuntimeFloatCurve Curve;

	FORCEINLINE operator FRuntimeFloatCurve&() { return Curve; }
	FORCEINLINE operator const FRuntimeFloatCurve&() const { return Curve; }

	[[nodiscard]] UCurveFloat* GetCurveAsset() const;

	[[nodiscard]] const FRichCurve* GetRichCurve() const;
	[[nodiscard]] FRichCurve* GetRichCurve();

	[[nodiscard]] bool HasAnyData() const;
	[[nodiscard]] float GetValue(float Time, float Default = 0.0f) const;

	void GetTimeRange(float& Min, float& Max) const;
	void GetValueRange(float& Min, float& Max) const;

	void ResetCurve();
	void RemovePoint(float Time);
	void AddOrUpdatePoint(float Time, float Value,
		ERichCurveTangentMode Tangent = RCTM_Auto);
};

/**
 * A Vector curve using inline keys or an assigned external curve asset.
 * Queries prefer the external asset when assigned. Editing helpers modify inline keys only
 * and do nothing while an external asset is assigned.
 */
USTRUCT(BlueprintType)
struct TOROCORE_API FInlineVectorCurve final
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = InlineCurve)
	FRuntimeVectorCurve Curve;

	FORCEINLINE operator FRuntimeVectorCurve&() { return Curve; }
	FORCEINLINE operator const FRuntimeVectorCurve&() const { return Curve; }

	[[nodiscard]] UCurveVector* GetCurveAsset() const;

	[[nodiscard]] const FRichCurve* GetRichCurve(uint8 Idx) const;
	[[nodiscard]] FRichCurve* GetRichCurve(uint8 Idx);

	[[nodiscard]] bool HasAnyData() const;
	[[nodiscard]] FVector GetValue(float Time, const FVector& Default = FVector::ZeroVector) const;

	void GetTimeRange(float& Min, float& Max) const;
	void GetValueRange(FVector& Min, FVector& Max) const;

	void ResetCurve();
	void RemovePoint(float Time);
	void AddOrUpdatePoint(float Time, const FVector& Value,
		ERichCurveTangentMode Tangent = RCTM_Auto);
};

/**
 * A Color curve using inline keys or an assigned external curve asset.
 * Queries prefer the external asset when assigned. Editing helpers modify inline keys only
 * and do nothing while an external asset is assigned.
 * Evaluation reads raw RGBA channels without applying asset color-adjustment settings.
 */
USTRUCT(BlueprintType)
struct TOROCORE_API FInlineColorCurve final
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = InlineCurve)
	FRuntimeCurveLinearColor Curve;

	FORCEINLINE operator FRuntimeCurveLinearColor&() { return Curve; }
	FORCEINLINE operator const FRuntimeCurveLinearColor&() const { return Curve; }

	[[nodiscard]] UCurveLinearColor* GetCurveAsset() const;

	[[nodiscard]] const FRichCurve* GetRichCurve(uint8 Idx) const;
	[[nodiscard]] FRichCurve* GetRichCurve(uint8 Idx);

	[[nodiscard]] bool HasAnyData() const;
	[[nodiscard]] FLinearColor GetValue(float Time, const FLinearColor& Default = FLinearColor::Black) const;

	void GetTimeRange(float& Min, float& Max) const;
	void GetValueRange(FLinearColor& Min, FLinearColor& Max) const;

	void ResetCurve();
	void RemovePoint(float Time);
	void AddOrUpdatePoint(float Time, const FLinearColor& Value,
		ERichCurveTangentMode Tangent = RCTM_Auto);
};

/**
 * Blueprint queries and inline-key editing helpers for float, vector, and color curves.
 * Queries prefer assigned external assets; editing helpers leave asset-backed curves unchanged.
 */
UCLASS(NotBlueprintable, NotBlueprintType)
class TOROCORE_API UInlineCurvesLibrary final : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/** Returns the assigned external curve asset, or nullptr when using inline data. */
	UFUNCTION(BlueprintPure, Category = "Math|Curves|Inline", DisplayName = "Get Curve Asset (Float)")
	static UCurveFloat* GetInlineCurveAsset_Float(const FInlineFloatCurve& Target);

	/** Returns whether the active curve has keys or a configured default in any channel. */
	UFUNCTION(BlueprintPure, Category = "Math|Curves|Inline", DisplayName = "Has Any Data (Float)")
	static bool HasInlineCurveData_Float(const FInlineFloatCurve& Target);

	/**
	 * Evaluates the active curve using its interpolation and extrapolation settings.
	 * @param Time Curve input time.
	 * @param Default Fallback for each channel with neither keys nor a configured curve default.
	 * Configured curve defaults take precedence over this fallback.
	 */
	UFUNCTION(BlueprintPure, Category = "Math|Curves|Inline", DisplayName = "Get Value (Float)")
	static float GetInlineCurveValue_Float(const FInlineFloatCurve& Target, float Time, float Default = 0.0f);

	/**
	 * Returns the earliest and latest key times across the active curve channels.
	 * Channels without keys do not contribute, even if they have configured defaults.
	 * @param Min Earliest key time, or zero when there are no keys.
	 * @param Max Latest key time, or zero when there are no keys.
	 */
	UFUNCTION(BlueprintPure, Category = "Math|Curves|Inline", DisplayName = "Get Time Range (Float)")
	static void GetInlineCurveTimeRange_Float(const FInlineFloatCurve& Target, float& Min, float& Max);

	/**
	 * Returns engine-computed value bounds for each active curve channel.
	 * Channels without keys return zero bounds, regardless of configured defaults.
	 * @param Min Lower value bounds.
	 * @param Max Upper value bounds.
	 */
	UFUNCTION(BlueprintPure, Category = "Math|Curves|Inline", DisplayName = "Get Value Range (Float)")
	static void GetInlineCurveValueRange_Float(const FInlineFloatCurve& Target, float& Min, float& Max);

	/**
	 * Removes inline keys from all channels, preserving defaults and extrapolation settings.
	 * Does nothing when an external curve asset is assigned.
	 */
	UFUNCTION(BlueprintCallable, Category = "Math|Curves|Inline", DisplayName = "Reset Curve (Float)")
	static void ResetInlineCurve_Float(UPARAM(ref) FInlineFloatCurve& Target);

	/**
	 * Removes a matching inline key from each channel; missing keys are ignored.
	 * Does nothing when an external curve asset is assigned.
	 * @param Time Key time matched using Unreal default key-time tolerance.
	 */
	UFUNCTION(BlueprintCallable, Category = "Math|Curves|Inline", DisplayName = "Remove Point (Float)")
	static void RemoveInlineCurvePoint_Float(UPARAM(ref) FInlineFloatCurve& Target, float Time);

	/**
	 * Adds or updates inline keys in all channels using Unreal default key-time tolerance.
	 * Does nothing when an external curve asset is assigned.
	 * New keys use linear interpolation; existing keys retain their interpolation mode.
	 * @param Time Time at which to add or update keys.
	 * @param Value Value to store in the corresponding channels.
	 * @param Tangent Tangent mode to assign. It affects cubic interpolation only;
	 * setting tangent mode does not enable cubic interpolation.
	 */
	UFUNCTION(BlueprintCallable, Category = "Math|Curves|Inline", DisplayName = "Add Or Update Point (Float)")
	static void AddOrUpdateInlineCurvePoint_Float(UPARAM(ref) FInlineFloatCurve& Target,
		float Time, float Value, TEnumAsByte<ERichCurveTangentMode> Tangent);

	/** Returns the assigned external curve asset, or nullptr when using inline data. */
	UFUNCTION(BlueprintPure, Category = "Math|Curves|Inline", DisplayName = "Get Curve Asset (Vector)")
	static UCurveVector* GetInlineCurveAsset_Vector(const FInlineVectorCurve& Target);

	/** Returns whether the active curve has keys or a configured default in any channel. */
	UFUNCTION(BlueprintPure, Category = "Math|Curves|Inline", DisplayName = "Has Any Data (Vector)")
	static bool HasInlineCurveData_Vector(const FInlineVectorCurve& Target);

	/**
	 * Evaluates the active curve using its interpolation and extrapolation settings.
	 * @param Time Curve input time.
	 * @param Default Fallback for each channel with neither keys nor a configured curve default.
	 * Configured curve defaults take precedence over this fallback.
	 */
	UFUNCTION(BlueprintPure, Category = "Math|Curves|Inline", DisplayName = "Get Value (Vector)")
	static FVector GetInlineCurveValue_Vector(const FInlineVectorCurve& Target, float Time, const FVector& Default);

	/**
	 * Returns the earliest and latest key times across the active curve channels.
	 * Channels without keys do not contribute, even if they have configured defaults.
	 * @param Min Earliest key time, or zero when there are no keys.
	 * @param Max Latest key time, or zero when there are no keys.
	 */
	UFUNCTION(BlueprintPure, Category = "Math|Curves|Inline", DisplayName = "Get Time Range (Vector)")
	static void GetInlineCurveTimeRange_Vector(const FInlineVectorCurve& Target, float& Min, float& Max);

	/**
	 * Returns engine-computed value bounds for each active curve channel.
	 * Channels without keys return zero bounds, regardless of configured defaults.
	 * @param Min Lower value bounds.
	 * @param Max Upper value bounds.
	 */
	UFUNCTION(BlueprintPure, Category = "Math|Curves|Inline", DisplayName = "Get Value Range (Vector)")
	static void GetInlineCurveValueRange_Vector(const FInlineVectorCurve& Target, FVector& Min, FVector& Max);

	/**
	 * Removes inline keys from all channels, preserving defaults and extrapolation settings.
	 * Does nothing when an external curve asset is assigned.
	 */
	UFUNCTION(BlueprintCallable, Category = "Math|Curves|Inline", DisplayName = "Reset Curve (Vector)")
	static void ResetInlineCurve_Vector(UPARAM(ref) FInlineVectorCurve& Target);

	/**
	 * Removes a matching inline key from each channel; missing keys are ignored.
	 * Does nothing when an external curve asset is assigned.
	 * @param Time Key time matched using Unreal default key-time tolerance.
	 */
	UFUNCTION(BlueprintCallable, Category = "Math|Curves|Inline", DisplayName = "Remove Point (Vector)")
	static void RemoveInlineCurvePoint_Vector(UPARAM(ref) FInlineVectorCurve& Target, float Time);

	/**
	 * Adds or updates inline keys in all channels using Unreal default key-time tolerance.
	 * Does nothing when an external curve asset is assigned.
	 * New keys use linear interpolation; existing keys retain their interpolation mode.
	 * @param Time Time at which to add or update keys.
	 * @param Value Value to store in the corresponding channels.
	 * @param Tangent Tangent mode to assign. It affects cubic interpolation only;
	 * setting tangent mode does not enable cubic interpolation.
	 */
	UFUNCTION(BlueprintCallable, Category = "Math|Curves|Inline", DisplayName = "Add Or Update Point (Vector)")
	static void AddOrUpdateInlineCurvePoint_Vector(UPARAM(ref) FInlineVectorCurve& Target,
		float Time, const FVector& Value, TEnumAsByte<ERichCurveTangentMode> Tangent);

	/** Returns the assigned external curve asset, or nullptr when using inline data. */
	UFUNCTION(BlueprintPure, Category = "Math|Curves|Inline", DisplayName = "Get Curve Asset (Color)")
	static UCurveLinearColor* GetInlineCurveAsset_Color(const FInlineColorCurve& Target);

	/** Returns whether the active curve has keys or a configured default in any channel. */
	UFUNCTION(BlueprintPure, Category = "Math|Curves|Inline", DisplayName = "Has Any Data (Color)")
	static bool HasInlineCurveData_Color(const FInlineColorCurve& Target);

	/**
	 * Evaluates the active curve using its interpolation and extrapolation settings.
	 * @param Time Curve input time.
	 * @param Default Fallback for each channel with neither keys nor a configured curve default.
	 * Configured curve defaults take precedence over this fallback.
	 * Returns raw RGBA values without applying asset color-adjustment settings.
	 */
	UFUNCTION(BlueprintPure, Category = "Math|Curves|Inline", DisplayName = "Get Value (Color)")
	static FLinearColor GetInlineCurveValue_Color(const FInlineColorCurve& Target, float Time, const FLinearColor& Default);

	/**
	 * Returns the earliest and latest key times across the active curve channels.
	 * Channels without keys do not contribute, even if they have configured defaults.
	 * @param Min Earliest key time, or zero when there are no keys.
	 * @param Max Latest key time, or zero when there are no keys.
	 */
	UFUNCTION(BlueprintPure, Category = "Math|Curves|Inline", DisplayName = "Get Time Range (Color)")
	static void GetInlineCurveTimeRange_Color(const FInlineColorCurve& Target, float& Min, float& Max);

	/**
	 * Returns engine-computed value bounds for each active curve channel.
	 * Channels without keys return zero bounds, regardless of configured defaults.
	 * @param Min Lower value bounds.
	 * @param Max Upper value bounds.
	 */
	UFUNCTION(BlueprintPure, Category = "Math|Curves|Inline", DisplayName = "Get Value Range (Color)")
	static void GetInlineCurveValueRange_Color(const FInlineColorCurve& Target, FLinearColor& Min, FLinearColor& Max);

	/**
	 * Removes inline keys from all channels, preserving defaults and extrapolation settings.
	 * Does nothing when an external curve asset is assigned.
	 */
	UFUNCTION(BlueprintCallable, Category = "Math|Curves|Inline", DisplayName = "Reset Curve (Color)")
	static void ResetInlineCurve_Color(UPARAM(ref) FInlineColorCurve& Target);

	/**
	 * Removes a matching inline key from each channel; missing keys are ignored.
	 * Does nothing when an external curve asset is assigned.
	 * @param Time Key time matched using Unreal default key-time tolerance.
	 */
	UFUNCTION(BlueprintCallable, Category = "Math|Curves|Inline", DisplayName = "Remove Point (Color)")
	static void RemoveInlineCurvePoint_Color(UPARAM(ref) FInlineColorCurve& Target, float Time);

	/**
	 * Adds or updates inline keys in all channels using Unreal default key-time tolerance.
	 * Does nothing when an external curve asset is assigned.
	 * New keys use linear interpolation; existing keys retain their interpolation mode.
	 * @param Time Time at which to add or update keys.
	 * @param Value Value to store in the corresponding channels.
	 * @param Tangent Tangent mode to assign. It affects cubic interpolation only;
	 * setting tangent mode does not enable cubic interpolation.
	 */
	UFUNCTION(BlueprintCallable, Category = "Math|Curves|Inline", DisplayName = "Add Or Update Point (Color)")
	static void AddOrUpdateInlineCurvePoint_Color(UPARAM(ref) FInlineColorCurve& Target,
		float Time, const FLinearColor& Value, TEnumAsByte<ERichCurveTangentMode> Tangent);
};
