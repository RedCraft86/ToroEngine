// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "CurvePlayers/BaseCurvePlayerComponent.h"

UBaseCurvePlayerComponent::UBaseCurvePlayerComponent()
	: bLooping(false)
	, PlayRate(1.0f)
	, CurrentTime(0.0f)
	, TimeRange(FVector2D::ZeroVector)
	, PlayState(ECurvePlayerState::Stopped)
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UBaseCurvePlayerComponent::StopPlaying(bool bReset)
{
	const bool bStateChanged = PlayState != ECurvePlayerState::Stopped;
	PlayState = ECurvePlayerState::Stopped;
	SetComponentTickEnabled(false);
	if (bReset)
	{
		SetPlaybackTime(TimeRange.X);
	}

	if (bStateChanged)
	{
		BroadcastState();
	}
}

void UBaseCurvePlayerComponent::PlayForward(bool bFromStart)
{
	if (!IsValidCurve())
	{
		StopPlaying(true);
	}
	else if (PlayState != ECurvePlayerState::Forward || bFromStart)
	{
		const bool bStateChanged = PlayState != ECurvePlayerState::Forward;
		PlayState = ECurvePlayerState::Forward;
		SetComponentTickEnabled(true);
		if (bStateChanged)
		{
			BroadcastState();
		}

		// If explicitly set OR if the animation cannot loop, but has already played
		if (bFromStart || (IsAtEnd() && !bLooping))
		{
			SetPlaybackTime(TimeRange.X);
		}
	}
}

void UBaseCurvePlayerComponent::PlayReverse(bool bFromEnd)
{
	if (!IsValidCurve())
	{
		StopPlaying(true);
	}
	else if (PlayState != ECurvePlayerState::Reverse || bFromEnd)
	{
		const bool bStateChanged = PlayState != ECurvePlayerState::Reverse;
		PlayState = ECurvePlayerState::Reverse;
		SetComponentTickEnabled(true);
		if (bStateChanged)
		{
			BroadcastState();
		}

		// If explicitly set OR if the animation cannot loop, but has already reversed
		if (bFromEnd || (IsAtStart() && !bLooping))
		{
			SetPlaybackTime(TimeRange.Y);
		}
	}
}

void UBaseCurvePlayerComponent::SetPlaybackTime(float InTime, bool bNotify)
{
	if (!IsValidCurve())
	{
		return;
	}

	const float ClampedTime = FMath::Clamp(InTime, TimeRange.X, TimeRange.Y);
	if (!FMath::IsNearlyEqual(CurrentTime, ClampedTime))
	{
		CurrentTime = ClampedTime;
		if (bNotify)
		{
			BroadcastEval();
		}
	}
}

void UBaseCurvePlayerComponent::BroadcastState() const
{
	OnStateChangedNative.Broadcast(PlayState);
	OnStateChanged.Broadcast(PlayState);
}

void UBaseCurvePlayerComponent::BeginPlay()
{
	Super::BeginPlay();
	InitializeCurve();
}

void UBaseCurvePlayerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* Func)
{
	Super::TickComponent(DeltaTime, TickType, Func);
	if (PlayState == ECurvePlayerState::Stopped || !IsValidCurve())
	{
		SetComponentTickEnabled(false);
		return;
	}

	const float StartTime = TimeRange.X;
	const float EndTime = TimeRange.Y;
	const float Duration = EndTime - StartTime;
	const bool bReverse = PlayState == ECurvePlayerState::Reverse;
	const float Direction = bReverse ? -1.0f : 1.0f;
	const float Delta = PlayRate * DeltaTime * Direction;
	const float TargetTime = CurrentTime + Delta;

	if (bLooping)
	{
		if (TargetTime < StartTime)
		{
			const float Overflow = FMath::Fmod(StartTime - TargetTime, Duration);
			CurrentTime = EndTime - Overflow;
		}
		else if (TargetTime > EndTime)
		{
			const float Overflow = FMath::Fmod(TargetTime - EndTime, Duration);
			CurrentTime = StartTime + Overflow;
		}
		else
		{
			CurrentTime = TargetTime;
		}
	}
	else
	{
		CurrentTime = FMath::Clamp(TargetTime, StartTime, EndTime);
		const bool bReachedDestination = bReverse ? TargetTime <= StartTime : TargetTime >= EndTime;
		const bool bOutsideRange = TargetTime < StartTime || TargetTime > EndTime;
		if (bReachedDestination || bOutsideRange)
		{
			StopPlaying(false);
		}
	}

	BroadcastEval();
}
