// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#include "Libraries/ToroCameraLibrary.h"
#include "Components/SceneCaptureComponent2D.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "Engine/GameViewportClient.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Utilities/WorldGetter.h"
#include "Engine/LocalPlayer.h"
#include "ConvexVolume.h"
#include "CoreGlobals.h"
#include "SceneView.h"
#if WITH_EDITOR
#include "Editor.h"
#include "EditorViewportClient.h"
#include "Misc/App.h"
#endif

namespace
{
	/**
	 * Tests world-space actor bounds against the frustum calculated from a minimal camera view.
	 */
	bool IsActorInCameraView(const FMinimalViewInfo& ViewInfo, const AActor& TestActor)
	{
		FMatrix ViewMatrix, ProjMatrix, ViewProjMatrix;
		UGameplayStatics::CalculateViewProjectionMatricesFromMinimalView(
			ViewInfo, TOptional<FMatrix>(),
			ViewMatrix, ProjMatrix, ViewProjMatrix
		);

		FConvexVolume Frustum;
		GetViewFrustumBounds(Frustum, ViewProjMatrix, true);

		FVector Origin, Extent;
		TestActor.GetActorBounds(false, Origin, Extent, true);

		return Frustum.IntersectBox(Origin, Extent);
	}
}

bool UToroCameraLibrary::IsActorInCameraFrustum(UCameraComponent* Target, const AActor* TestActor)
{
	if (!IsValid(Target) || !IsValid(TestActor))
	{
		return false;
	}

	FMinimalViewInfo ViewInfo;
	Target->GetCameraView(0.0f, ViewInfo);
	return IsActorInCameraView(ViewInfo, *TestActor);
}

bool UToroCameraLibrary::IsActorInSceneCapture2DFrustum(USceneCaptureComponent2D* Target, const AActor* TestActor)
{
	if (!IsValid(Target) || !IsValid(TestActor))
	{
		return false;
	}

	FMinimalViewInfo ViewInfo;
	Target->GetCameraView(0.0f, ViewInfo);
	return IsActorInCameraView(ViewInfo, *TestActor);
}

bool UToroCameraLibrary::IsActorInViewFrustum(const AActor* TestActor, int32 PlayerIdx)
{
	if (!IsValid(TestActor))
	{
		return false;
	}

	const APlayerController* PC = UGameplayStatics::GetPlayerController(FWorldGetter::Get(TestActor), PlayerIdx);
	const ULocalPlayer* LP = IsValid(PC) ? PC->GetLocalPlayer() : nullptr;
	if (!IsValid(LP) || !IsValid(LP->ViewportClient) || !LP->ViewportClient->Viewport)
	{
		return false;
	}

	FSceneViewProjectionData ProjData;
	if (!LP->GetProjectionData(LP->ViewportClient->Viewport, ProjData))
	{
		return false;
	}

	FConvexVolume Frustum;
	GetViewFrustumBounds(Frustum, ProjData.ComputeViewProjectionMatrix(), true);

	FVector Origin, Extent;
	TestActor->GetActorBounds(false, Origin, Extent, true);

	return Frustum.IntersectBox(Origin, Extent);
}

FTransform UToroCameraLibrary::GetViewTransform(const UObject* ContextObject, int32 PlayerIdx)
{
	static TMap<int32, TFrameValue<FTransform>> IndexToTransform;

#if WITH_EDITOR
	TFrameValue<FTransform>& OutTransform = IndexToTransform.FindOrAdd(FApp::IsGame() ? PlayerIdx : -1);
#else
	TFrameValue<FTransform>& OutTransform = IndexToTransform.FindOrAdd(PlayerIdx);
#endif

	if (OutTransform.IsSet())
	{
		return OutTransform.GetValue();
	}

	OutTransform = FTransform::Identity;

#if WITH_EDITOR
	if (!FApp::IsGame() && GEditor)
	{
		const FViewport* VP = GEditor->GetActiveViewport();
		if (const FEditorViewportClient* VPC = VP ? static_cast<FEditorViewportClient*>(VP->GetClient()) : nullptr)
		{
			OutTransform = FTransform(
				VPC->GetViewRotation(),
				VPC->GetViewLocation(),
				FVector::OneVector
			);
		}

		return OutTransform.GetValue();
	}
#endif

	const APlayerCameraManager* PCM = UGameplayStatics::GetPlayerCameraManager(FWorldGetter::Get(ContextObject), PlayerIdx);
	if (IsValid(PCM))
	{
		OutTransform = FTransform(
			PCM->GetCameraRotation(),
			PCM->GetCameraLocation(),
			FVector::OneVector
		);
	}

	return OutTransform.GetValue();
}

bool UToroCameraLibrary::StopCameraFade(const UObject* ContextObject, int32 PlayerIdx)
{
	APlayerCameraManager* PCM = UGameplayStatics::GetPlayerCameraManager(FWorldGetter::Get(ContextObject), PlayerIdx);
	if (IsValid(PCM))
	{
		PCM->StopCameraFade();
		return true;
	}

	return false;
}

bool UToroCameraLibrary::SetCameraFade(const UObject* ContextObject, FLinearColor Color,
	float Alpha, bool bFadeAudio, int32 PlayerIdx)
{
	APlayerCameraManager* PCM = UGameplayStatics::GetPlayerCameraManager(FWorldGetter::Get(ContextObject), PlayerIdx);
	if (IsValid(PCM))
	{
		PCM->SetManualCameraFade(Alpha, Color, bFadeAudio);
		return true;
	}

	return false;
}

FVoidCoroutine UToroCameraLibrary::StartCameraFade(FLatentActionInfo LatentInfo, const UObject* ContextObject,
	bool& bSuccess, FLinearColor Color, float Duration, float FromAlpha,
	float ToAlpha, bool bFadeAudio, bool bHoldAtEnd, int32 PlayerIdx)
{
	bSuccess = false;
	if (!FMath::IsFinite(Duration))
	{
		co_return;
	}

	const float AdjustedTime = FMath::Max(0.0f, Duration);
	APlayerCameraManager* PCM = UGameplayStatics::GetPlayerCameraManager(FWorldGetter::Get(ContextObject), PlayerIdx);
	if (IsValid(PCM))
	{
		PCM->StartCameraFade(FromAlpha, ToAlpha, AdjustedTime, Color, bFadeAudio, bHoldAtEnd);
		if (AdjustedTime != 0.0f)
		{
			co_await UE5Coro::Latent::Seconds(AdjustedTime);
		}

		bSuccess = true;
	}

	co_return;
}

AActor* UToroCameraLibrary::GetPlayerViewTarget(const UObject* ContextObject, int32 PlayerIdx)
{
	const APlayerController* PC = UGameplayStatics::GetPlayerController(FWorldGetter::Get(ContextObject), PlayerIdx);
	return IsValid(PC) ? PC->GetViewTarget() : nullptr;
}

bool UToroCameraLibrary::SetPlayerViewTarget(const UObject* ContextObject, AActor* NewTarget, int32 PlayerIdx)
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(FWorldGetter::Get(ContextObject), PlayerIdx);
	if (IsValid(PC))
	{
		PC->SetViewTarget(NewTarget);
		return true;
	}

	return false;
}

FVoidCoroutine UToroCameraLibrary::BlendPlayerViewTarget(FLatentActionInfo LatentInfo, const UObject* ContextObject,
	bool& bSuccess, AActor* NewTarget, float Duration, EViewTargetBlendFunction BlendFunc,
	float BlendExp, bool bLockOutgoing, int32 PlayerIdx)
{
	bSuccess = false;
	if (!FMath::IsFinite(Duration))
	{
		co_return;
	}

	const float AdjustedTime = FMath::Max(0.0f, Duration);
	APlayerController* PC = UGameplayStatics::GetPlayerController(FWorldGetter::Get(ContextObject), PlayerIdx);
	if (IsValid(PC))
	{
		PC->SetViewTargetWithBlend(NewTarget, AdjustedTime, BlendFunc, BlendExp, bLockOutgoing);
		if (AdjustedTime != 0.0f)
		{
			co_await UE5Coro::Latent::Seconds(AdjustedTime);
		}

		bSuccess = true;
	}

	co_return;
}
