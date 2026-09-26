// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#include "DataTypes/InlineCurves.h"

namespace
{
	constexpr uint8 NumVector	= 3;
	constexpr uint8 NumColor	= 4;
}

UCurveFloat* FInlineFloatCurve::GetCurveAsset() const
{
	return Curve.ExternalCurve;
}

const FRichCurve* FInlineFloatCurve::GetRichCurve() const
{
	return Curve.GetRichCurveConst();
}

FRichCurve* FInlineFloatCurve::GetRichCurve()
{
	return Curve.GetRichCurve();
}

bool FInlineFloatCurve::HasAnyData() const
{
	return GetRichCurve()->HasAnyData();
}

float FInlineFloatCurve::GetValue(const float Time, const float Default) const
{
	return GetRichCurve()->Eval(Time, Default);
}

void FInlineFloatCurve::GetTimeRange(float& Min, float& Max) const
{
	GetRichCurve()->GetTimeRange(Min, Max);
}

void FInlineFloatCurve::GetValueRange(float& Min, float& Max) const
{
	GetRichCurve()->GetValueRange(Min, Max);
}

void FInlineFloatCurve::ResetCurve()
{
	if (!GetCurveAsset())
	{
		GetRichCurve()->Reset();
	}
}

void FInlineFloatCurve::RemovePoint(const float Time)
{
	if (!GetCurveAsset())
	{
		FRichCurve* CurvePtr = GetRichCurve();
		if (const FKeyHandle Point = CurvePtr->FindKey(Time))
		{
			CurvePtr->DeleteKey(Point);
		}
	}
}

void FInlineFloatCurve::AddOrUpdatePoint(const float Time,
	const float Value, const ERichCurveTangentMode Tangent)
{
	if (!GetCurveAsset())
	{
		FRichCurve* CurvePtr = GetRichCurve();
		if (const FKeyHandle Point = CurvePtr->UpdateOrAddKey(Time, Value))
		{
			CurvePtr->SetKeyTangentMode(Point, Tangent);
		}
	}
}

UCurveVector* FInlineVectorCurve::GetCurveAsset() const
{
	return Curve.ExternalCurve;
}

const FRichCurve* FInlineVectorCurve::GetRichCurve(const uint8 Idx) const
{
	checkf(Idx < NumVector,
		TEXT("FInlineVectorCurve::GetRichCurve only accepts indices [0,3). Provided value %d is invalid."), Idx
	);

	return Curve.GetRichCurveConst(Idx);
}

FRichCurve* FInlineVectorCurve::GetRichCurve(const uint8 Idx)
{
	checkf(Idx < NumVector,
		TEXT("FInlineVectorCurve::GetRichCurve only accepts indices [0,3). Provided value %d is invalid."), Idx
	);

	return Curve.GetRichCurve(Idx);
}

bool FInlineVectorCurve::HasAnyData() const
{
	for (uint8 i = 0; i < NumVector; i++)
	{
		if (GetRichCurve(i)->HasAnyData())
		{
			return true;
		}
	}

	return false;
}

FVector FInlineVectorCurve::GetValue(const float Time, const FVector& Default) const
{
	FVector Result = Default;
	for (uint8 i = 0; i < NumVector; i++)
	{
		Result.Component(i) = GetRichCurve(i)->Eval(Time, Default.Component(i));
	}

	return Result;
}

void FInlineVectorCurve::GetTimeRange(float& Min, float& Max) const
{
	Min = Max = 0.0f;
	bool bHasKeys = false;
	for (uint8 i = 0; i < NumVector; i++)
	{
		const FRichCurve* CurvePtr = GetRichCurve(i);
		if (CurvePtr->GetNumKeys() > 0)
		{
			float CurveMin, CurveMax;
			CurvePtr->GetTimeRange(CurveMin, CurveMax);

			if (bHasKeys)
			{
				Min = FMath::Min(Min, CurveMin);
				Max = FMath::Max(Max, CurveMax);
			}
			else
			{
				Min = CurveMin;
				Max = CurveMax;
			}

			bHasKeys = true;
		}
	}
}

void FInlineVectorCurve::GetValueRange(FVector& Min, FVector& Max) const
{
	for (uint8 i = 0; i < NumVector; i++)
	{
		float CurveMin, CurveMax;
		GetRichCurve(i)->GetValueRange(CurveMin, CurveMax);
		Min.Component(i) = CurveMin;
		Max.Component(i) = CurveMax;
	}
}

void FInlineVectorCurve::ResetCurve()
{
	if (!GetCurveAsset())
	{
		for (uint8 i = 0; i < NumVector; i++)
		{
			GetRichCurve(i)->Reset();
		}
	}
}

void FInlineVectorCurve::RemovePoint(const float Time)
{
	if (!GetCurveAsset())
	{
		for (uint8 i = 0; i < NumVector; i++)
		{
			FRichCurve* CurvePtr = GetRichCurve(i);
			if (const FKeyHandle Point = CurvePtr->FindKey(Time))
			{
				CurvePtr->DeleteKey(Point);
			}
		}
	}
}

void FInlineVectorCurve::AddOrUpdatePoint(const float Time,
	const FVector& Value, const ERichCurveTangentMode Tangent)
{
	if (!GetCurveAsset())
	{
		for (uint8 i = 0; i < NumVector; i++)
		{
			FRichCurve* CurvePtr = GetRichCurve(i);
			if (const FKeyHandle Point = CurvePtr->UpdateOrAddKey(Time, Value.Component(i)))
			{
				CurvePtr->SetKeyTangentMode(Point, Tangent);
			}
		}
	}
}

UCurveLinearColor* FInlineColorCurve::GetCurveAsset() const
{
	return Curve.ExternalCurve;
}

const FRichCurve* FInlineColorCurve::GetRichCurve(const uint8 Idx) const
{
	checkf(Idx < NumColor,
		TEXT("FInlineColorCurve::GetRichCurve only accepts indices [0,4). Provided value %d is invalid."), Idx
	);

	return Curve.ExternalCurve ? &Curve.ExternalCurve->FloatCurves[Idx] : &Curve.ColorCurves[Idx];
}

FRichCurve* FInlineColorCurve::GetRichCurve(const uint8 Idx)
{
	checkf(Idx < NumColor,
		TEXT("FInlineColorCurve::GetRichCurve only accepts indices [0,4). Provided value %d is invalid."), Idx
	);

	return Curve.ExternalCurve ? &Curve.ExternalCurve->FloatCurves[Idx] : &Curve.ColorCurves[Idx];
}

bool FInlineColorCurve::HasAnyData() const
{
	for (uint8 i = 0; i < NumColor; i++)
	{
		if (GetRichCurve(i)->HasAnyData())
		{
			return true;
		}
	}

	return false;
}

FLinearColor FInlineColorCurve::GetValue(const float Time, const FLinearColor& Default) const
{
	FLinearColor Result = Default;
	for (uint8 i = 0; i < NumColor; i++)
	{
		Result.Component(i) = GetRichCurve(i)->Eval(Time, Default.Component(i));
	}

	return Result;
}

void FInlineColorCurve::GetTimeRange(float& Min, float& Max) const
{
	Min = Max = 0.0f;
	bool bHasKeys = false;
	for (uint8 i = 0; i < NumColor; i++)
	{
		const FRichCurve* CurvePtr = GetRichCurve(i);
		if (CurvePtr->GetNumKeys() > 0)
		{
			float CurveMin, CurveMax;
			CurvePtr->GetTimeRange(CurveMin, CurveMax);

			if (bHasKeys)
			{
				Min = FMath::Min(Min, CurveMin);
				Max = FMath::Max(Max, CurveMax);
			}
			else
			{
				Min = CurveMin;
				Max = CurveMax;
			}

			bHasKeys = true;
		}
	}
}

void FInlineColorCurve::GetValueRange(FLinearColor& Min, FLinearColor& Max) const
{
	for (uint8 i = 0; i < NumColor; i++)
	{
		float CurveMin, CurveMax;
		GetRichCurve(i)->GetValueRange(CurveMin, CurveMax);
		Min.Component(i) = CurveMin;
		Max.Component(i) = CurveMax;
	}
}

void FInlineColorCurve::ResetCurve()
{
	if (!GetCurveAsset())
	{
		for (uint8 i = 0; i < NumColor; i++)
		{
			GetRichCurve(i)->Reset();
		}
	}
}

void FInlineColorCurve::RemovePoint(const float Time)
{
	if (!GetCurveAsset())
	{
		for (uint8 i = 0; i < NumColor; i++)
		{
			FRichCurve* CurvePtr = GetRichCurve(i);
			if (const FKeyHandle Point = CurvePtr->FindKey(Time))
			{
				CurvePtr->DeleteKey(Point);
			}
		}
	}
}

void FInlineColorCurve::AddOrUpdatePoint(const float Time,
	const FLinearColor& Value, const ERichCurveTangentMode Tangent)
{
	if (!GetCurveAsset())
	{
		for (uint8 i = 0; i < NumColor; i++)
		{
			FRichCurve* CurvePtr = GetRichCurve(i);
			if (const FKeyHandle Point = CurvePtr->UpdateOrAddKey(Time, Value.Component(i)))
			{
				CurvePtr->SetKeyTangentMode(Point, Tangent);
			}
		}
	}
}

UCurveFloat* UInlineCurvesLibrary::GetInlineCurveAsset_Float(const FInlineFloatCurve& Target)
{
	return Target.GetCurveAsset();
}

bool UInlineCurvesLibrary::HasInlineCurveData_Float(const FInlineFloatCurve& Target)
{
	return Target.HasAnyData();
}

float UInlineCurvesLibrary::GetInlineCurveValue_Float(const FInlineFloatCurve& Target, const float Time, const float Default)
{
	return Target.GetValue(Time, Default);
}

void UInlineCurvesLibrary::GetInlineCurveTimeRange_Float(const FInlineFloatCurve& Target, float& Min, float& Max)
{
	Target.GetTimeRange(Min, Max);
}

void UInlineCurvesLibrary::GetInlineCurveValueRange_Float(const FInlineFloatCurve& Target, float& Min, float& Max)
{
	Target.GetValueRange(Min, Max);
}

void UInlineCurvesLibrary::ResetInlineCurve_Float(FInlineFloatCurve& Target)
{
	Target.ResetCurve();
}

void UInlineCurvesLibrary::RemoveInlineCurvePoint_Float(FInlineFloatCurve& Target, const float Time)
{
	Target.RemovePoint(Time);
}

void UInlineCurvesLibrary::AddOrUpdateInlineCurvePoint_Float(FInlineFloatCurve& Target,
	const float Time, const float Value, const TEnumAsByte<ERichCurveTangentMode> Tangent)
{
	Target.AddOrUpdatePoint(Time, Value, Tangent);
}

UCurveVector* UInlineCurvesLibrary::GetInlineCurveAsset_Vector(const FInlineVectorCurve& Target)
{
	return Target.GetCurveAsset();
}

bool UInlineCurvesLibrary::HasInlineCurveData_Vector(const FInlineVectorCurve& Target)
{
	return Target.HasAnyData();
}

FVector UInlineCurvesLibrary::GetInlineCurveValue_Vector(const FInlineVectorCurve& Target, const float Time, const FVector& Default)
{
	return Target.GetValue(Time, Default);
}

void UInlineCurvesLibrary::GetInlineCurveTimeRange_Vector(const FInlineVectorCurve& Target, float& Min, float& Max)
{
	Target.GetTimeRange(Min, Max);
}

void UInlineCurvesLibrary::GetInlineCurveValueRange_Vector(const FInlineVectorCurve& Target, FVector& Min, FVector& Max)
{
	Target.GetValueRange(Min, Max);
}

void UInlineCurvesLibrary::ResetInlineCurve_Vector(FInlineVectorCurve& Target)
{
	return Target.ResetCurve();
}

void UInlineCurvesLibrary::RemoveInlineCurvePoint_Vector(FInlineVectorCurve& Target, const float Time)
{
	Target.RemovePoint(Time);
}

void UInlineCurvesLibrary::AddOrUpdateInlineCurvePoint_Vector(FInlineVectorCurve& Target,
	const float Time, const FVector& Value, const TEnumAsByte<ERichCurveTangentMode> Tangent)
{
	Target.AddOrUpdatePoint(Time, Value, Tangent);
}

UCurveLinearColor* UInlineCurvesLibrary::GetInlineCurveAsset_Color(const FInlineColorCurve& Target)
{
	return Target.GetCurveAsset();
}

bool UInlineCurvesLibrary::HasInlineCurveData_Color(const FInlineColorCurve& Target)
{
	return Target.HasAnyData();
}

FLinearColor UInlineCurvesLibrary::GetInlineCurveValue_Color(const FInlineColorCurve& Target, const float Time, const FLinearColor& Default)
{
	return Target.GetValue(Time, Default);
}

void UInlineCurvesLibrary::GetInlineCurveTimeRange_Color(const FInlineColorCurve& Target, float& Min, float& Max)
{
	Target.GetTimeRange(Min, Max);
}

void UInlineCurvesLibrary::GetInlineCurveValueRange_Color(const FInlineColorCurve& Target, FLinearColor& Min, FLinearColor& Max)
{
	Target.GetValueRange(Min, Max);
}

void UInlineCurvesLibrary::ResetInlineCurve_Color(FInlineColorCurve& Target)
{
	Target.ResetCurve();
}

void UInlineCurvesLibrary::RemoveInlineCurvePoint_Color(FInlineColorCurve& Target, const float Time)
{
	Target.RemovePoint(Time);
}

void UInlineCurvesLibrary::AddOrUpdateInlineCurvePoint_Color(FInlineColorCurve& Target,
	const float Time, const FLinearColor& Value, const TEnumAsByte<ERichCurveTangentMode> Tangent)
{
	Target.AddOrUpdatePoint(Time, Value, Tangent);
}
