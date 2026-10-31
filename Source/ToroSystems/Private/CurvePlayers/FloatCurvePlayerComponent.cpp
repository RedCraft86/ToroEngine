// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "CurvePlayers/FloatCurvePlayerComponent.h"

UFloatCurvePlayerComponent::UFloatCurvePlayerComponent()
{
	Curve.AddOrUpdatePoint(0.0f, 0.0f);
	Curve.AddOrUpdatePoint(1.0f, 1.0f);
}

void UFloatCurvePlayerComponent::SetCurve(const FInlineFloatCurve& InCurve)
{
	if (InCurve.HasAnyData())
	{
		StopPlaying(false);

		Curve = InCurve;
		InitializeCurve();

		BroadcastEval();
	}
}

void UFloatCurvePlayerComponent::InitializeCurve()
{
	float Max;
	Curve.GetTimeRange(CurrentTime, Max);
	TimeRange = FVector2D(CurrentTime, Max);
}

void UFloatCurvePlayerComponent::BroadcastEval() const
{
	const float Value = GetValue();
	OnEvaluateNative.Broadcast(Value, CurrentTime);
	OnEvaluate.Broadcast(Value, CurrentTime);
}
