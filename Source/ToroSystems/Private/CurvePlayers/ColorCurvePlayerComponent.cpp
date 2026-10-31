// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "CurvePlayers/ColorCurvePlayerComponent.h"

UColorCurvePlayerComponent::UColorCurvePlayerComponent()
{
	Curve.AddOrUpdatePoint(0.0f, FLinearColor::Black);
	Curve.AddOrUpdatePoint(1.0f, FLinearColor::White);
}

void UColorCurvePlayerComponent::SetCurve(const FInlineColorCurve& InCurve)
{
	if (InCurve.HasAnyData())
	{
		StopPlaying(false);

		Curve = InCurve;
		InitializeCurve();

		BroadcastEval();
	}
}

void UColorCurvePlayerComponent::InitializeCurve()
{
	float Max;
	Curve.GetTimeRange(CurrentTime, Max);
	TimeRange = FVector2D(CurrentTime, Max);
}

void UColorCurvePlayerComponent::BroadcastEval() const
{
	const FLinearColor Value = GetValue();
	OnEvaluateNative.Broadcast(Value, CurrentTime);
	OnEvaluate.Broadcast(Value, CurrentTime);
}
