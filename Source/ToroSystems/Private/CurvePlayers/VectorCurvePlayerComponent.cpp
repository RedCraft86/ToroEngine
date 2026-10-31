// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "CurvePlayers/VectorCurvePlayerComponent.h"

UVectorCurvePlayerComponent::UVectorCurvePlayerComponent()
{
	Curve.AddOrUpdatePoint(0.0f, FVector::ZeroVector);
	Curve.AddOrUpdatePoint(1.0f, FVector::OneVector);
}

void UVectorCurvePlayerComponent::SetCurve(const FInlineVectorCurve& InCurve)
{
	if (InCurve.HasAnyData())
	{
		StopPlaying(false);

		Curve = InCurve;
		InitializeCurve();

		BroadcastEval();
	}
}

void UVectorCurvePlayerComponent::InitializeCurve()
{
	float Max;
	Curve.GetTimeRange(CurrentTime, Max);
	TimeRange = FVector2D(CurrentTime, Max);
}

void UVectorCurvePlayerComponent::BroadcastEval() const
{
	const FVector Value = GetValue();
	OnEvaluateNative.Broadcast(Value, CurrentTime);
	OnEvaluate.Broadcast(Value, CurrentTime);
}
