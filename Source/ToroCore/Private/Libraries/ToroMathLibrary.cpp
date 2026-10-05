// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

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

double UToroMathLibrary::GetHorizontalDistance(FVector A, FVector B)
{
	return FVector::DistXY(A, B);
}

FLinearColor UToroMathLibrary::TemperatureToLinearColor(float Temperature)
{
	return FLinearColor::MakeFromColorTemperature(Temperature);
}

FLinearColor UToroMathLibrary::RandomLinearColor(bool bTrueRandom, bool bRandomAlpha)
{
	FLinearColor Result;
	if (bTrueRandom)
	{
		Result.R = FMath::FRand();
		Result.G = FMath::FRand();
		Result.B = FMath::FRand();
	}
	else
	{
		Result = FLinearColor::MakeRandomColor();
	}

	Result.A = bRandomAlpha ? FMath::FRand() : 1.0f;
	return Result;
}

FColor UToroMathLibrary::TemperatureToColor(float Temperature)
{
	return FColor::MakeFromColorTemperature(Temperature);
}

FColor UToroMathLibrary::RandomColor(bool bTrueRandom, bool bRandomAlpha)
{
	FColor Result;
	if (bTrueRandom)
	{
		Result.R = static_cast<uint8>(FMath::RandRange(0, 255));
		Result.G = static_cast<uint8>(FMath::RandRange(0, 255));
		Result.B = static_cast<uint8>(FMath::RandRange(0, 255));
	}
	else
	{
		Result = FColor::MakeRandomColor();
	}

	Result.A = bRandomAlpha ? static_cast<uint8>(FMath::RandRange(0, 255)) : 255;
	return Result;
}

float UToroMathLibrary::PerlinNoise1D(float Position)
{
	return FMath::PerlinNoise1D(Position);
}

float UToroMathLibrary::PerlinNoise2D(FVector2D Position)
{
	return FMath::PerlinNoise2D(Position);
}

float UToroMathLibrary::PerlinNoise3D(FVector Position)
{
	return FMath::PerlinNoise3D(Position);
}
