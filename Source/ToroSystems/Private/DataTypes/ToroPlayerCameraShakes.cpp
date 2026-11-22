// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "DataTypes/ToroPlayerCameraShakes.h"
#include "Shakes/WaveOscillatorCameraShakePattern.h"

namespace
{
	// Legacy oscillators use radians per second. Wave oscillators use cycles per second.
	constexpr float LegacyFrequencyToHz = 1.0f / (2.0f * UE_PI);

	void ConfigureMovementPattern(UWaveOscillatorCameraShakePattern& Pattern)
	{
		Pattern.Duration = 0.0f;
		Pattern.BlendInTime = 0.18f;
		Pattern.X.Amplitude = 0.0f;
		Pattern.Y.Amplitude = 0.0f;
		Pattern.FOV.Amplitude = 0.0f;
		Pattern.Z.InitialOffsetType = EInitialWaveOscillatorOffsetType::Zero;
		Pattern.Pitch.InitialOffsetType = EInitialWaveOscillatorOffsetType::Zero;
		Pattern.Yaw.InitialOffsetType = EInitialWaveOscillatorOffsetType::Zero;
		Pattern.Roll.InitialOffsetType = EInitialWaveOscillatorOffsetType::Zero;
	}
}

UToroWalkCameraShake::UToroWalkCameraShake(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	auto* Pattern = CreateDefaultSubobject<UWaveOscillatorCameraShakePattern>(TEXT("WalkPattern"));
	ConfigureMovementPattern(*Pattern);
	Pattern->Pitch.Amplitude = 0.65f;
	Pattern->Pitch.Frequency = 12.0f * LegacyFrequencyToHz;
	Pattern->Yaw.Amplitude = 0.3f;
	Pattern->Yaw.Frequency = 6.0f * LegacyFrequencyToHz;
	Pattern->Roll.Amplitude = 0.35f;
	Pattern->Roll.Frequency = 3.0f * LegacyFrequencyToHz;
	Pattern->Z.Amplitude = 1.75f;
	Pattern->Z.Frequency = 12.0f * LegacyFrequencyToHz;
	SetRootShakePattern(Pattern);
}

UToroRunCameraShake::UToroRunCameraShake(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	auto* Pattern = CreateDefaultSubobject<UWaveOscillatorCameraShakePattern>(TEXT("RunPattern"));
	ConfigureMovementPattern(*Pattern);
	Pattern->Pitch.Amplitude = 1.2f;
	Pattern->Pitch.Frequency = 17.0f * LegacyFrequencyToHz;
	Pattern->Yaw.Amplitude = 0.5f;
	Pattern->Yaw.Frequency = 8.5f * LegacyFrequencyToHz;
	Pattern->Roll.Amplitude = 0.55f;
	Pattern->Roll.Frequency = 4.75f * LegacyFrequencyToHz;
	Pattern->Z.Amplitude = 4.0f;
	Pattern->Z.Frequency = 17.0f * LegacyFrequencyToHz;
	SetRootShakePattern(Pattern);
}
