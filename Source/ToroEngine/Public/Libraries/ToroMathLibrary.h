// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "ToroMathLibrary.generated.h"

/**
 * Blueprint math utilities for constants, signs, planar distance, colors, and Perlin noise, with native array reductions.
 */
UCLASS(NotBlueprintable, NotBlueprintType)
class TOROENGINE_API UToroMathLibrary final : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/** Gets UE_SMALL_NUMBER, the engine's small floating-point tolerance (1e-8). */
	UFUNCTION(BlueprintPure, Category = "Math|Constant", meta = (CompactNodeTitle = "SMALL"))
	[[nodiscard]] static float SmallNumber();

	/** Gets UE_KINDA_SMALL_NUMBER, the engine's larger floating-point tolerance (1e-4). */
	UFUNCTION(BlueprintPure, Category = "Math|Constant", meta = (CompactNodeTitle = "KINDA SMALL"))
	[[nodiscard]] static float KindaSmallNumber();

	/** Gets UE_BIG_NUMBER, the engine's large finite float constant (3.4e38). */
	UFUNCTION(BlueprintPure, Category = "Math|Constant", meta = (CompactNodeTitle = "BIG"))
	[[nodiscard]] static float BigNumber();

	/**
	 * Gets the absolute magnitude of the input.
	 * @param InValue Value whose sign is removed.
	 */
	UFUNCTION(BlueprintPure, Category = "Math|float", meta = (CompactNodeTitle = "POSITIVE"))
	[[nodiscard]] static float MakePositive(float InValue);

	/**
	 * Gets the negated absolute magnitude of the input.
	 * @param InValue Value whose magnitude is returned with a negative sign, including negative zero for zero.
	 */
	UFUNCTION(BlueprintPure, Category = "Math|float", meta = (CompactNodeTitle = "NEGATIVE"))
	[[nodiscard]] static float MakeNegative(float InValue);

	/**
	 * Computes distance in the XY plane, ignoring differences in Z.
	 * @param A First position.
	 * @param B Second position.
	 */
	UFUNCTION(BlueprintPure, Category = "Math|Vector")
	[[nodiscard]] static double GetHorizontalDistance(FVector A, FVector B);

	/**
	 * Converts a color temperature to linear RGB using the engine's Planckian-locus approximation.
	 * @param Temperature Temperature in Kelvin, clamped by the native function to 1000 through 15000 K.
	 */
	UFUNCTION(BlueprintPure, Category = "Math|LinearColor")
	[[nodiscard]] static FLinearColor TemperatureToLinearColor(float Temperature);

	/**
	 * Generates a linear RGB color using a native hue sequence or independent channel samples.
	 * @param bRandomHue True uses the engine's shared stepped hue sequence at full saturation and value,
	 * rather than independently sampling a random hue; false independently samples R, G, and B in [0, 1].
	 * @param bRandomAlpha Whether to sample alpha in [0, 1]; otherwise alpha is 1.
	 */
	UFUNCTION(BlueprintPure, Category = "Math|LinearColor")
	[[nodiscard]] static FLinearColor RandomLinearColor(bool bRandomHue = true, bool bRandomAlpha = false);

	/**
	 * Converts a color temperature to an 8-bit sRGB color through the native linear-color conversion.
	 * @param Temperature Temperature in Kelvin, clamped by the native function to 1000 through 15000 K.
	 */
	UFUNCTION(BlueprintPure, Category = "Math|Color")
	[[nodiscard]] static FColor TemperatureToColor(float Temperature);

	/**
	 * Generates an 8-bit color using the native saturated hue sequence or independent RGB byte samples.
	 * @param bRandomHue True converts the engine's shared stepped hue sequence to sRGB,
	 * rather than independently sampling a random hue; false independently samples each RGB byte from 0 through 255.
	 * @param bRandomAlpha Whether to sample alpha from 0 through 255; otherwise alpha is 255.
	 */
	UFUNCTION(BlueprintPure, Category = "Math|Color")
	[[nodiscard]] static FColor RandomColor(bool bRandomHue = true, bool bRandomAlpha = false);

	/**
	 * Samples deterministic one-dimensional Perlin noise in the native range [-1, 1].
	 * @param Position Finite coordinate to sample; repeated coordinates produce the same value.
	 */
	UFUNCTION(BlueprintPure, Category = "Math|Random")
	[[nodiscard]] static float PerlinNoise1D(float Position);

	/**
	 * Samples deterministic two-dimensional Perlin noise in the native range [-1, 1].
	 * @param Position Finite XY coordinates to sample; repeated coordinates produce the same value.
	 */
	UFUNCTION(BlueprintPure, Category = "Math|Random")
	[[nodiscard]] static float PerlinNoise2D(FVector2D Position);

	/**
	 * Samples deterministic three-dimensional Perlin noise in the native range [-1, 1].
	 * @param Position Finite XYZ coordinates to sample; repeated coordinates produce the same value.
	 */
	UFUNCTION(BlueprintPure, Category = "Math|Random")
	[[nodiscard]] static float PerlinNoise3D(FVector Position);

	/**
	 * Folds array elements from left to right, starting with BaseValue.
	 * @tparam T Value type supporting copying and assignment of the accumulator and callable results.
	 * @param InArray Elements visited in index order.
	 * @param BaseValue Initial accumulator, also returned when the array is empty.
	 * @param Func Valid callable receiving the current accumulator and next element by const reference, returning the next T value.
	 */
	template <typename T>
	[[nodiscard]] static T ArrayOperate(const TArray<T>& InArray, const T& BaseValue, const TFunction<T(const T&, const T&)>& Func)
	{
		T Result = BaseValue;
		for (int32 i = 0; i < InArray.Num(); i++)
		{
			Result = Func(Result, InArray[i]);
		}
		return Result;
	}

	/**
	 * Adds array elements from left to right, starting with BaseValue.
	 * @tparam T Value type supporting copying, assignment, and addition with a result convertible to T.
	 *         Must also be constructible from 0 when the default BaseValue is used.
	 * @param InArray Elements to sum in index order.
	 * @param BaseValue Initial sum, defaulting to T(0); returned unchanged for an empty array.
	 */
	template <typename T>
	[[nodiscard]] static T ArraySum(const TArray<T>& InArray, const T& BaseValue = T(0))
	{
		return ArrayOperate<T>(InArray, BaseValue, [](const T& A, const T& B)
			{
				return A + B;
			});
	}

	/**
	 * Multiplies array elements from left to right, starting with BaseValue.
	 * @tparam T Value type supporting copying, assignment, and multiplication with a result convertible to T.
	 *         Must also be constructible from 1 when the default BaseValue is used.
	 * @param InArray Elements to multiply in index order.
	 * @param BaseValue Initial product, defaulting to T(1); returned unchanged for an empty array.
	 */
	template <typename T>
	[[nodiscard]] static T ArrayProduct(const TArray<T>& InArray, const T& BaseValue = T(1))
	{
		return ArrayOperate<T>(InArray, BaseValue, [](const T& A, const T& B)
			{
				return A * B;
			});
	}
};
