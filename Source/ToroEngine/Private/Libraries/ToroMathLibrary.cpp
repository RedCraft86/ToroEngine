// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "Libraries/ToroMathLibrary.h"

float UToroMathLibrary::SmallNumber()
{
	return UE_SMALL_NUMBER;
}

float UToroMathLibrary::KindaSmallNumber()
{
	return UE_KINDA_SMALL_NUMBER;
}

float UToroMathLibrary::BigNumber()
{
	return UE_BIG_NUMBER;
}

float UToroMathLibrary::MakePositive(float InValue)
{
	return FMath::Abs(InValue);
}

float UToroMathLibrary::MakeNegative(float InValue)
{
	return -FMath::Abs(InValue);
}

double UToroMathLibrary::GetHorizontalDistance(FVector A, FVector B)
{
	return FVector::DistXY(A, B);
}

FLinearColor UToroMathLibrary::TemperatureToLinearColor(float Temperature)
{
	return FLinearColor::MakeFromColorTemperature(Temperature);
}

FLinearColor UToroMathLibrary::RandomLinearColor(bool bRandomHue, bool bRandomAlpha)
{
	FLinearColor Result;
	if (bRandomHue)
	{
		Result = FLinearColor::MakeRandomColor();
	}
	else
	{
		Result.R = FMath::FRand();
		Result.G = FMath::FRand();
		Result.B = FMath::FRand();
	}

	Result.A = bRandomAlpha ? FMath::FRand() : 1.0f;
	return Result;
}

FColor UToroMathLibrary::TemperatureToColor(float Temperature)
{
	return FColor::MakeFromColorTemperature(Temperature);
}

FColor UToroMathLibrary::RandomColor(bool bRandomHue, bool bRandomAlpha)
{
	FColor Result;
	if (bRandomHue)
	{
		Result = FColor::MakeRandomColor();
	}
	else
	{
		Result.R = static_cast<uint8>(FMath::RandRange(0, 255));
		Result.G = static_cast<uint8>(FMath::RandRange(0, 255));
		Result.B = static_cast<uint8>(FMath::RandRange(0, 255));
	}

	Result.A = bRandomAlpha ? static_cast<uint8>(FMath::RandRange(0, 255)) : 255;
	return Result;
}

float UToroMathLibrary::PerlinNoise2D(FVector2D Position)
{
	return FMath::PerlinNoise2D(Position);
}

float UToroMathLibrary::PerlinNoise3D(FVector Position)
{
	return FMath::PerlinNoise3D(Position);
}
