// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "DataTypes/InlineCurves.h"
#include "Curves/RichCurve.h"

namespace
{
	constexpr uint8 NumVector = 3;
	constexpr uint8 NumColor = 4;
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

float FInlineFloatCurve::GetValue(float Time, float Default) const
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

void FInlineFloatCurve::RemovePoint(float Time)
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

void FInlineFloatCurve::AddOrUpdatePoint(float Time, float Value, ERichCurveTangentMode Tangent)
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

const FRichCurve* FInlineVectorCurve::GetRichCurve(uint8 Idx) const
{
	checkf(Idx < NumVector, TEXT("FInlineVectorCurve::GetRichCurve only accepts indices [0..3]. %d is invalid."), Idx);
	return Curve.GetRichCurveConst(Idx);
}

FRichCurve* FInlineVectorCurve::GetRichCurve(uint8 Idx)
{
	checkf(Idx < NumVector, TEXT("FInlineVectorCurve::GetRichCurve only accepts indices [0..3]. %d is invalid."), Idx);
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

FVector FInlineVectorCurve::GetValue(float Time, const FVector& Default) const
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

void FInlineVectorCurve::RemovePoint(float Time)
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

void FInlineVectorCurve::AddOrUpdatePoint(float Time, const FVector& Value, ERichCurveTangentMode Tangent)
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

const FRichCurve* FInlineColorCurve::GetRichCurve(uint8 Idx) const
{
	checkf(Idx < NumColor, TEXT("FInlineColorCurve::GetRichCurve only accepts indices [0..4]. %d is invalid."), Idx);
	return Curve.ExternalCurve ? &Curve.ExternalCurve->FloatCurves[Idx] : &Curve.ColorCurves[Idx];
}

FRichCurve* FInlineColorCurve::GetRichCurve(uint8 Idx)
{
	checkf(Idx < NumColor, TEXT("FInlineColorCurve::GetRichCurve only accepts indices [0..4]. %d is invalid."), Idx);
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

FLinearColor FInlineColorCurve::GetValue(float Time, const FLinearColor& Default) const
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

void FInlineColorCurve::RemovePoint(float Time)
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

void FInlineColorCurve::AddOrUpdatePoint(float Time, const FLinearColor& Value, ERichCurveTangentMode Tangent)
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

UCurveFloat* UInlineCurveLibrary::GetFloatCurveAsset(const FInlineFloatCurve& Curve)
{
	return Curve.Curve.ExternalCurve;
}

bool UInlineCurveLibrary::HasFloatCurveData(const FInlineFloatCurve& Curve)
{
	return Curve.HasAnyData();
}

float UInlineCurveLibrary::GetFloatCurveValue(const FInlineFloatCurve& Curve, float Time, float DefaultValue)
{
	return Curve.GetValue(Time, DefaultValue);
}

void UInlineCurveLibrary::GetFloatCurveTimeRange(const FInlineFloatCurve& Curve, float& Min, float& Max)
{
	Curve.GetTimeRange(Min, Max);
}

void UInlineCurveLibrary::GetFloatCurveValueRange(const FInlineFloatCurve& Curve, float& Min, float& Max)
{
	Curve.GetValueRange(Min, Max);
}

void UInlineCurveLibrary::ResetFloatCurve(FInlineFloatCurve& Curve)
{
	Curve.ResetCurve();
}

void UInlineCurveLibrary::RemoveFloatCurvePoint(FInlineFloatCurve& Curve, float Time)
{
	Curve.RemovePoint(Time);
}

void UInlineCurveLibrary::AddOrUpdateFloatCurvePoint(FInlineFloatCurve& Curve, float Time, float Value, TEnumAsByte<ERichCurveTangentMode> Tangent)
{
	Curve.AddOrUpdatePoint(Time, Value, Tangent.GetValue());
}

UCurveVector* UInlineCurveLibrary::GetVectorCurveAsset(const FInlineVectorCurve& Curve)
{
	return Curve.Curve.ExternalCurve;
}

bool UInlineCurveLibrary::HasVectorCurveData(const FInlineVectorCurve& Curve)
{
	return Curve.HasAnyData();
}

FVector UInlineCurveLibrary::GetVectorCurveValue(const FInlineVectorCurve& Curve, float Time, FVector DefaultValue)
{
	return Curve.GetValue(Time, DefaultValue);
}

void UInlineCurveLibrary::GetVectorCurveTimeRange(const FInlineVectorCurve& Curve, float& Min, float& Max)
{
	Curve.GetTimeRange(Min, Max);
}

void UInlineCurveLibrary::GetVectorCurveValueRange(const FInlineVectorCurve& Curve, FVector& Min, FVector& Max)
{
	Curve.GetValueRange(Min, Max);
}

void UInlineCurveLibrary::ResetVectorCurve(FInlineVectorCurve& Curve)
{
	Curve.ResetCurve();
}

void UInlineCurveLibrary::RemoveVectorCurvePoint(FInlineVectorCurve& Curve, float Time)
{
	Curve.RemovePoint(Time);
}

void UInlineCurveLibrary::AddOrUpdateVectorCurvePoint(FInlineVectorCurve& Curve, float Time, FVector Value, TEnumAsByte<ERichCurveTangentMode> Tangent)
{
	Curve.AddOrUpdatePoint(Time, Value, Tangent.GetValue());
}

UCurveLinearColor* UInlineCurveLibrary::GetColorCurveAsset(const FInlineColorCurve& Curve)
{
	return Curve.Curve.ExternalCurve;
}

bool UInlineCurveLibrary::HasColorCurveData(const FInlineColorCurve& Curve)
{
	return Curve.HasAnyData();
}

FLinearColor UInlineCurveLibrary::GetColorCurveValue(const FInlineColorCurve& Curve, float Time, FLinearColor DefaultValue)
{
	return Curve.GetValue(Time, DefaultValue);
}

void UInlineCurveLibrary::GetColorCurveTimeRange(const FInlineColorCurve& Curve, float& Min, float& Max)
{
	Curve.GetTimeRange(Min, Max);
}

void UInlineCurveLibrary::GetColorCurveValueRange(const FInlineColorCurve& Curve, FLinearColor& Min, FLinearColor& Max)
{
	Curve.GetValueRange(Min, Max);
}

void UInlineCurveLibrary::ResetColorCurve(FInlineColorCurve& Curve)
{
	Curve.ResetCurve();
}

void UInlineCurveLibrary::RemoveColorCurvePoint(FInlineColorCurve& Curve, float Time)
{
	Curve.RemovePoint(Time);
}

void UInlineCurveLibrary::AddOrUpdateColorCurvePoint(FInlineColorCurve& Curve, float Time, FLinearColor Value, TEnumAsByte<ERichCurveTangentMode> Tangent)
{
	Curve.AddOrUpdatePoint(Time, Value, Tangent.GetValue());
}
