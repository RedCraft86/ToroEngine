// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "LightProbes/LightProbeComponent.h"
#include "LightProbes/LightProbeSubsystem.h"
#if WITH_EDITOR
#include "Components/TintedBillboardComponent.h"
#include "UObject/UnrealType.h"
#include "UObject/ICookInfo.h"
#include "Engine/Texture2D.h"
#include "Engine/World.h"
#endif

ULightProbeComponent::ULightProbeComponent()
	: Intensity(1.0f)
	, Radius(500.0f)
	, Falloff(2.0f)
	, Color(FLinearColor::White)
	, bDisableWithLumenGI(false)
	, MaxDistance(3000.0f)
	, FadeRange(100.0f)
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	bAutoActivate = true;
}

bool ULightProbeComponent::IsProbeRelevant(const FTransform& CamTransform, bool bHasLumenGI) const
{
	if (!IsVisible()
		|| Radius < 50.0f
		|| Intensity < 0.1f
		|| (bHasLumenGI && bDisableWithLumenGI))
	{
		return false;
	}

	const FVector CameraToProbe = GetComponentLocation() - CamTransform.GetLocation();
	if (CameraToProbe.SizeSquared() > FMath::Square(MaxDistance))
	{
		return false;
	}

	const FVector CameraForward = CamTransform.GetRotation().Vector();
	return FVector::DotProduct(CameraForward, CameraToProbe) >= -Radius;
}

void ULightProbeComponent::GetProbeData(const FTransform& CamTransform, FVector4& PosData, FVector4& EffectData) const
{
	const FVector Location = GetComponentLocation();

	PosData = Location;
	PosData.W = Radius;

	float Multiplier = Intensity;
	if (FadeRange > 0.0f)
	{
		Multiplier *= FMath::GetMappedRangeValueClamped(
			FVector2D(MaxDistance - FadeRange, MaxDistance),
			FVector2D(1.0f, 0.0f),
			FVector::Dist(Location, CamTransform.GetLocation())
		);
	}

	EffectData = Color * Multiplier;
	EffectData.W = Falloff;
}

void ULightProbeComponent::OnRegister()
{
#if WITH_EDITORONLY_DATA
	bVisualizeComponent = false;
#endif
	Super::OnRegister();
#if WITH_EDITOR
	UpdateLightIcon();
#endif
	if (ULightProbeSubsystem* Manager = ULightProbeSubsystem::Get(this))
	{
		Manager->RegisterProbe(this);
	}
}

void ULightProbeComponent::OnUnregister()
{
	if (ULightProbeSubsystem* Manager = ULightProbeSubsystem::Get(this))
	{
		Manager->UnregisterProbe(this);
	}
	Super::OnUnregister();
}

#if WITH_EDITOR
void ULightProbeComponent::UpdateLightIcon()
{
	const UWorld* World = GetWorld();
	if (!IsValid(this) || IsTemplate() || !IsRegistered() || !IsValid(GetOwner()) || !World || World->IsGameWorld())
	{
		return;
	}

	if (SpriteComponent == nullptr)
	{
		// Create a new billboard component to serve as a visualization
		{
			FCookLoadScope EditorOnlyLoadScope(ECookLoadType::EditorOnly);

			const EObjectFlags TransactionalFlag = GetFlags() & RF_Transactional;
			SpriteComponent = NewObject<UTintedBillboardComponent>(
				GetOwner(), NAME_None, TransactionalFlag | RF_Transient | RF_TextExportTransient
			);

			SpriteComponent->Sprite = LoadObject<UTexture2D>(nullptr,
				TEXT("/ToroEngine/LightProbes/T_LightProbe.T_LightProbe")
			);
		}

		SpriteComponent->SetRelativeScale3D_Direct(FVector(0.5f, 0.5f, 0.5f));
		SpriteComponent->Mobility = EComponentMobility::Movable;
		SpriteComponent->AlwaysLoadOnClient = false;
		SpriteComponent->SetIsVisualizationComponent(true);
		SpriteComponent->SpriteInfo.Category = TEXT("LightProbe");
		SpriteComponent->SpriteInfo.DisplayName = NSLOCTEXT("SpriteCategory", "LightProbe", "LightProbe");
		SpriteComponent->CreationMethod = CreationMethod;
		SpriteComponent->bIsScreenSizeScaled = true;
		SpriteComponent->bUseInEditorScaling = true;
		SpriteComponent->OpacityMaskRefVal = 0.3f;

		SpriteComponent->SetupAttachment(this);
		SpriteComponent->RegisterComponent();
	}

	if (UTintedBillboardComponent* TintedSprite = Cast<UTintedBillboardComponent>(SpriteComponent))
	{
		TintedSprite->SetSpriteColor(Color);
	}
}

void ULightProbeComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (IsValid(this) && !IsTemplate() && IsRegistered()
		&& PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(ULightProbeComponent, Color))
	{
		UpdateLightIcon();
	}
}
#endif
