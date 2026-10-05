// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "ToroMathLibrary.generated.h"

/**
 * Numeric constants, color generation, Perlin noise, and ordered array reductions.
 */
UCLASS(NotBlueprintable, NotBlueprintType)
class TOROCORE_API UToroMathLibrary final : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/**
	 * Returns UE_SMALL_NUMBER (1.e-8), a small floating-point tolerance.
	 */
	UFUNCTION(BlueprintPure, Category = "Math|Constant", meta = (CompactNodeTitle = "Small"))
	[[nodiscard]] static float SmallNumber();

	/**
	 * Returns UE_KINDA_SMALL_NUMBER (1.e-4), a looser floating-point tolerance.
	 */
	UFUNCTION(BlueprintPure, Category = "Math|Constant", meta = (CompactNodeTitle = "Kinda Small"))
	[[nodiscard]] static float KindaSmallNumber();

	/**
	 * Returns UE_BIG_NUMBER (3.4e+38), a large finite float near FLT_MAX.
	 */
	UFUNCTION(BlueprintPure, Category = "Math|Constant", meta = (CompactNodeTitle = "Big"))
	[[nodiscard]] static float BigNumber();

	/**
	 * Returns distance in the XY plane, ignoring differences in Z.
	 * @param A First position.
	 * @param B Second position.
	 */
	UFUNCTION(BlueprintPure, Category = "Math|Vector")
	[[nodiscard]] static double GetHorizontalDistance(const FVector A, const FVector B);

	/**
	 * Converts a color temperature in kelvin to linear RGB with opaque alpha.
	 * @param Temperature Finite temperature; the engine clamps its supported range.
	 */
	UFUNCTION(BlueprintPure, Category = "Math|LinearColor")
	[[nodiscard]] static FLinearColor TemperatureToLinearColor(const float Temperature);

	/**
	 * Generates a linear color using pseudorandom channels or the engine's stepped hue sequence.
	 * @param bTrueRandom Use the engine RNG to sample RGB independently in 0..1; otherwise use a stepped hue at full saturation and value.
	 * @param bRandomAlpha Sample alpha in 0..1 using the engine RNG rather than use opaque alpha.
	 */
	UFUNCTION(BlueprintPure, Category = "Math|LinearColor")
	[[nodiscard]] static FLinearColor RandomLinearColor(const bool bTrueRandom, const bool bRandomAlpha = false);

	/**
	 * Converts a color temperature in kelvin to an opaque sRGB byte color.
	 * @param Temperature Finite temperature; the engine clamps its supported range.
	 */
	UFUNCTION(BlueprintPure, Category = "Math|Color")
	[[nodiscard]] static FColor TemperatureToColor(const float Temperature);

	/**
	 * Generates a byte color using pseudorandom channels or the engine's stepped hue sequence.
	 * @param bTrueRandom Use the engine RNG to sample RGB independently in 0..255; otherwise use the engine's stepped hue sequence.
	 * @param bRandomAlpha Sample alpha in 0..255 using the engine RNG rather than use opaque alpha (255).
	 */
	UFUNCTION(BlueprintPure, Category = "Math|Color")
	[[nodiscard]] static FColor RandomColor(const bool bTrueRandom, const bool bRandomAlpha = false);

	/**
	 * Samples deterministic continuous one-dimensional Perlin noise.
	 * @param Position Finite coordinate; no RNG state is consumed.
	 * @return Noise value in the engine's nominal -1..1 range.
	 */
	UFUNCTION(BlueprintPure, Category = "Math|Random")
	[[nodiscard]] static float PerlinNoise1D(const float Position);

	/**
	 * Samples deterministic continuous two-dimensional Perlin noise.
	 * @param Position Finite coordinates; no RNG state is consumed.
	 * @return Noise value in the engine's nominal -1..1 range.
	 */
	UFUNCTION(BlueprintPure, Category = "Math|Random")
	[[nodiscard]] static float PerlinNoise2D(const FVector2D Position);

	/**
	 * Samples deterministic continuous three-dimensional Perlin noise.
	 * @param Position Finite coordinates; no RNG state is consumed.
	 * @return Noise value in the engine's nominal -1..1 range.
	 */
	UFUNCTION(BlueprintPure, Category = "Math|Random")
	[[nodiscard]] static float PerlinNoise3D(const FVector Position);

	/**
	 * Folds elements in array order, passing the accumulator first to Func.
	 * @tparam T Element and accumulator type.
	 * @param InArray Elements to reduce.
	 * @param BaseValue Initial accumulator and result for an empty array.
	 * @param Func Valid callable taking the accumulator and next element and returning the next accumulator.
	 */
	template<typename T>
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
	 * Adds elements to an initial value in array order.
	 * @tparam T Type supporting construction from zero and addition.
	 * @param InArray Elements to sum.
	 * @param BaseValue Initial sum and result for an empty array.
	 */
	template<typename T>
	[[nodiscard]] static T ArraySum(const TArray<T>& InArray, const T& BaseValue = T(0))
	{
		return ArrayOperate<T>(InArray, BaseValue, [](const T& A, const T& B)
			{
				return A + B;
			});
	}

	/**
	 * Multiplies elements into an initial value in array order.
	 * @tparam T Type supporting construction from one and multiplication.
	 * @param InArray Elements to multiply.
	 * @param BaseValue Initial product and result for an empty array.
	 */
	template<typename T>
	[[nodiscard]] static T ArrayProduct(const TArray<T>& InArray, const T& BaseValue = T(1))
	{
		return ArrayOperate<T>(InArray, BaseValue, [](const T& A, const T& B)
			{
				return A * B;
			});
	}
};
