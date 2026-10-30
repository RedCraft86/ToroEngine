// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "Libraries/ToroCameraLibrary.h"
#include "Utilities/ToroUtilities.h"
#include "Utilities/ToroLogging.h"
#include "Components/SceneCaptureComponent2D.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Actor.h"
#include "Engine/GameViewportClient.h"
#include "Engine/LocalPlayer.h"
#include "Camera/PlayerCameraManager.h"
#include "Camera/CameraComponent.h"
#include "Camera/CameraTypes.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Misc/FrameValue.h"
#include "UnrealClient.h"
#include "ConvexVolume.h"
#include "SceneView.h"
#if WITH_EDITOR
#include "EditorViewportClient.h"
#include "Editor.h"
#endif

namespace
{
	bool IsActorInView(const FMinimalViewInfo& ViewInfo, const AActor& TestActor,
		const TOptional<FMatrix>& ProjectionMatrix = TOptional<FMatrix>())
	{
		FMatrix ViewMatrix, ProjMatrix, ViewProjMatrix;
		UGameplayStatics::CalculateViewProjectionMatricesFromMinimalView(
			ViewInfo, ProjectionMatrix,
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
	return IsActorInView(ViewInfo, *TestActor);
}

bool UToroCameraLibrary::IsActorInSceneCapture2DFrustum(USceneCaptureComponent2D* Target, const AActor* TestActor)
{
	if (!IsValid(Target) || !IsValid(TestActor))
	{
		return false;
	}

	FMinimalViewInfo ViewInfo;
	Target->GetCameraView(0.0f, ViewInfo);
	const TOptional<FMatrix> CustomProjection = Target->bUseCustomProjectionMatrix
		? TOptional<FMatrix>(Target->CustomProjectionMatrix) : TOptional<FMatrix>();
	return IsActorInView(ViewInfo, *TestActor, CustomProjection);
}

bool UToroCameraLibrary::IsActorInViewFrustum(const AActor* TestActor, int32 PlayerIdx)
{
	if (!IsValid(TestActor))
	{
		return false;
	}

	const APlayerController* PC = UGameplayStatics::GetPlayerController(TestActor, PlayerIdx);
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
	const UWorld* World = ToroEngine::GetWorld(ContextObject);
	if (!IsValid(World))
	{
		return FTransform::Identity;
	}

	int32 CacheKey = PlayerIdx;
#if WITH_EDITOR
	if (World->WorldType == EWorldType::Editor)
	{
		CacheKey = -1;
	}
	else
#endif
	if (!World->IsGameWorld() || PlayerIdx < 0)
	{
		return FTransform::Identity;
	}

	static TMap<int32, TFrameValue<FTransform>> IndexToTransform;
	TFrameValue<FTransform>& CachedTransform = IndexToTransform.FindOrAdd(CacheKey);
	if (CachedTransform.IsSet())
	{
		return CachedTransform.GetValue();
	}

#if WITH_EDITOR
	if (World->WorldType == EWorldType::Editor)
	{
		const FViewport* Viewport = GEditor ? GEditor->GetActiveViewport() : nullptr;
		if (const FViewportClient* ActiveClient = Viewport ? Viewport->GetClient() : nullptr)
		{
			for (const FEditorViewportClient* EditorClient : GEditor->GetAllViewportClients())
			{
				if (EditorClient && EditorClient == ActiveClient && EditorClient->GetWorld() == World)
				{
					CachedTransform = FTransform(
						EditorClient->GetViewRotation(),
						EditorClient->GetViewLocation(),
						FVector::OneVector
					);
					return CachedTransform.GetValue();
				}
			}
		}

		return FTransform::Identity;
	}
#endif

	const APlayerCameraManager* PCM = UGameplayStatics::GetPlayerCameraManager(World, PlayerIdx);
	if (IsValid(PCM))
	{
		CachedTransform = FTransform(
			PCM->GetCameraRotation(),
			PCM->GetCameraLocation(),
			FVector::OneVector
		);
		return CachedTransform.GetValue();
	}

	return FTransform::Identity;
}

bool UToroCameraLibrary::StopCameraFade(const UObject* ContextObject, int32 PlayerIdx)
{
	APlayerCameraManager* PCM = UGameplayStatics::GetPlayerCameraManager(ContextObject, PlayerIdx);
	if (IsValid(PCM))
	{
		PCM->StopCameraFade();
		return true;
	}

	return false;
}

bool UToroCameraLibrary::SetCameraFade(const UObject* ContextObject, FLinearColor Color, float Alpha, bool bFadeAudio, int32 PlayerIdx)
{
	APlayerCameraManager* PCM = UGameplayStatics::GetPlayerCameraManager(ContextObject, PlayerIdx);
	if (IsValid(PCM))
	{
		PCM->SetManualCameraFade(Alpha, Color, bFadeAudio);
		return true;
	}

	return false;
}

FVoidCoroutine UToroCameraLibrary::StartCameraFade(FLatentActionInfo LatentInfo, const UObject* ContextObject, bool& bSuccess,
	FLinearColor Color, float Duration, float FromAlpha, float ToAlpha, bool bFadeAudio, bool bHoldAtEnd, int32 PlayerIdx)
{
	bSuccess = false;

	if (!FMath::IsFinite(Duration))
	{
		TORO_REPORT_ERROR(TEXT("Duration must be finite."));
		co_return;
	}

	const float AdjustedDuration = FMath::Max(Duration, 0.0f);
	APlayerCameraManager* PCM = UGameplayStatics::GetPlayerCameraManager(ContextObject, PlayerIdx);
	if (IsValid(PCM))
	{
		if (AdjustedDuration > 0.0f)
		{
			PCM->StartCameraFade(FromAlpha, ToAlpha, AdjustedDuration, Color, bFadeAudio, bHoldAtEnd);
			co_await UE5Coro::Latent::Seconds(AdjustedDuration);
		}
		else
		{
			if (bHoldAtEnd)
			{
				PCM->SetManualCameraFade(ToAlpha, Color, bFadeAudio);
			}
			else
			{
				PCM->StopCameraFade();
			}
		}

		bSuccess = true;
	}
}

AActor* UToroCameraLibrary::GetPlayerViewTarget(const UObject* ContextObject, int32 PlayerIdx)
{
	const APlayerController* PC = UGameplayStatics::GetPlayerController(ContextObject, PlayerIdx);
	return IsValid(PC) ? PC->GetViewTarget() : nullptr;
}

bool UToroCameraLibrary::SetPlayerViewTarget(const UObject* ContextObject, AActor* NewTarget, int32 PlayerIdx)
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(ContextObject, PlayerIdx);
	if (IsValid(PC))
	{
		PC->SetViewTarget(NewTarget);
		return true;
	}

	return false;
}

FVoidCoroutine UToroCameraLibrary::BlendPlayerViewTarget(FLatentActionInfo LatentInfo, const UObject* ContextObject, bool& bSuccess,
	AActor* NewTarget, float Duration, EViewTargetBlendFunction BlendFunc, float BlendExp, bool bLockOutgoing, int32 PlayerIdx)
{
	bSuccess = false;

	if (!FMath::IsFinite(Duration))
	{
		TORO_REPORT_ERROR(TEXT("Duration must be finite."));
		co_return;
	}

	const float AdjustedDuration = FMath::Max(Duration, 0.0f);
	APlayerController* PC = UGameplayStatics::GetPlayerController(ContextObject, PlayerIdx);
	if (IsValid(PC))
	{
		PC->SetViewTargetWithBlend(NewTarget, AdjustedDuration, BlendFunc, BlendExp, bLockOutgoing);
		if (AdjustedDuration > 0.0f)
		{
			co_await UE5Coro::Latent::Seconds(AdjustedDuration);
		}

		bSuccess = true;
	}
}
