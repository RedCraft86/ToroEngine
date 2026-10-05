// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#pragma once

#include "UE5Coro.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ToroCameraLibrary.generated.h"

class UCameraComponent;
class USceneCaptureComponent2D;

/**
 * Camera-frustum tests, view transforms, fades, and player view-target helpers.
 * Operations using world lookup require the game thread and may use FWorldGetter fallback.
 * Frustum tests measure bounds intersection, not occlusion or actual rendered visibility.
 */
UCLASS(NotBlueprintable, NotBlueprintType)
class TOROCORE_API UToroCameraLibrary final : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/**
	 * Tests actor bounds against the camera's view frustum; this does not test occlusion.
	 * @param Target Camera whose view is sampled.
	 * @param TestActor Actor bounds to test, including child actors.
	 * @return True for intersecting bounds; false for invalid inputs or no intersection.
	 */
	UFUNCTION(BlueprintPure, Category = Camera, meta = (DefaultToSelf = Target))
	[[nodiscard]] static bool IsActorInCameraFrustum(UCameraComponent* Target, const AActor* TestActor);

	/**
	 * Tests actor bounds against the scene capture's minimal camera view; no occlusion test is performed.
	 * Custom projection matrices are not applied by this helper.
	 * @param Target Scene capture whose view is sampled.
	 * @param TestActor Actor bounds to test, including child actors.
	 * @return True for intersecting bounds; false for invalid inputs or no intersection.
	 */
	UFUNCTION(BlueprintPure, Category = SceneCapture, DisplayName = "Is Actor in Scene Capture 2D Frustum", meta = (DefaultToSelf = Target))
	[[nodiscard]] static bool IsActorInSceneCapture2DFrustum(USceneCaptureComponent2D* Target, const AActor* TestActor);

	/**
	 * Tests actor bounds against a local player's viewport projection; this does not test occlusion.
	 * @param TestActor Actor whose bounds and world context are used.
	 * @param PlayerIdx Player-controller index, defaulting to zero.
	 * @return True for intersecting bounds; false for invalid inputs, unavailable local projection, or no intersection.
	 */
	UFUNCTION(BlueprintPure, Category = PlayerView, meta = (DefaultToSelf = TestActor))
	[[nodiscard]] static bool IsActorInViewFrustum(const AActor* TestActor, const int32 PlayerIdx = 0);

	/**
	 * Returns camera location and rotation with unit scale, or identity when unavailable.
	 * Caches each resolved camera manager or editor viewport independently for the current frame.
	 * Editor application mode uses the active editor viewport instead of the supplied player.
	 * @param PlayerIdx Player-camera index used outside the editor viewport path.
	 */
	UFUNCTION(BlueprintCallable, Category = PlayerView, meta = (WorldContext = ContextObject, Keywords = "view point"))
	[[nodiscard]] static FTransform GetViewTransform(const UObject* ContextObject, const int32 PlayerIdx = 0);

	/**
	 * Stops fading on the selected player camera manager.
	 * @param PlayerIdx Player-camera index.
	 * @return True when a camera manager was found and notified; false otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = PlayerViewFade, meta = (WorldContext = ContextObject, Keywords = "fade camera"))
	static bool StopCameraFade(const UObject* ContextObject, const int32 PlayerIdx = 0);

	/**
	 * Sets a manual fade on the selected player camera manager.
	 * @param Color Fade color.
	 * @param Alpha Fade opacity, forwarded to the engine; callers should supply 0..1.
	 * @param bFadeAudio Also apply the fade to audio.
	 * @param PlayerIdx Player-camera index.
	 * @return True when a camera manager was found and notified; false otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = PlayerViewFade, meta = (WorldContext = ContextObject, Keywords = "fade camera"))
	static bool SetCameraFade(const UObject* ContextObject, const FLinearColor Color = FLinearColor::Black,
		const float Alpha = 1.0f, const bool bFadeAudio = true, const int32 PlayerIdx = 0);

	/**
	 * Starts a camera fade and waits for its configured duration, not confirmation of completion.
	 * @param bSuccess False for missing camera managers or nonfinite duration; true after the duration wait.
	 * @param Color Fade color.
	 * @param Duration Finite duration in seconds, clamped to zero.
	 * @param FromAlpha Initial opacity, normally 0..1.
	 * @param ToAlpha Final opacity, normally 0..1.
	 * @param bFadeAudio Also fade audio.
	 * @param bHoldAtEnd Keep the final fade after its duration.
	 * @param PlayerIdx Player-camera index.
	 */
	UFUNCTION(BlueprintCallable, Category = PlayerViewFade, meta = (Latent, LatentInfo = LatentInfo, WorldContext = ContextObject,
		Keywords = "fade camera", AdvancedDisplay = "FromAlpha, ToAlpha, bFadeAudio, bHoldAtEnd"))
	static FVoidCoroutine StartCameraFade(FLatentActionInfo LatentInfo, const UObject* ContextObject, bool& bSuccess,
		const FLinearColor Color = FLinearColor::Black, const float Duration = 1.0f, const float FromAlpha = 0.0f,
		const float ToAlpha = 1.0f, const bool bFadeAudio = true, const bool bHoldAtEnd = true, const int32 PlayerIdx = 0);

	/**
	 * Returns the selected controller's current view target, or nullptr when no controller is found.
	 * @param PlayerIdx Player-controller index.
	 */
	UFUNCTION(BlueprintCallable, Category = "Game|Player", meta = (WorldContext = ContextObject, Keywords = "get view camera"))
	static AActor* GetPlayerViewTarget(const UObject* ContextObject, const int32 PlayerIdx = 0);

	/**
	 * Requests an immediate view-target change.
	 * @param NewTarget Actor to view; nullptr uses the engine's controller fallback.
	 * @param PlayerIdx Player-controller index.
	 * @return True when a controller was found and notified; false otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = "Game|Player", meta = (WorldContext = ContextObject, Keywords = "set view camera"))
	static bool SetPlayerViewTarget(const UObject* ContextObject, AActor* NewTarget, const int32 PlayerIdx = 0);

	/**
	 * Requests a view-target blend and waits its duration, even if the blend is interrupted.
	 * @param bSuccess False for missing controllers or nonfinite duration; true after the duration wait.
	 * @param NewTarget Actor to view; nullptr uses the engine's controller fallback.
	 * @param Duration Finite blend duration in seconds, clamped to zero.
	 * @param BlendFunc Engine interpolation function.
	 * @param BlendExp Exponent for blend functions that use one.
	 * @param bLockOutgoing Freeze the outgoing view during the blend.
	 * @param PlayerIdx Player-controller index.
	 */
	UFUNCTION(BlueprintCallable, Category = "Game|Player", meta = (Latent, LatentInfo = LatentInfo, WorldContext = ContextObject,
		DefaultToSelf = NewTarget, Keywords = "blend view camera", AdvancedDisplay = "BlendFunc, BlendExp, bLockOutgoing"))
	static FVoidCoroutine BlendPlayerViewTarget(FLatentActionInfo LatentInfo, const UObject* ContextObject, bool& bSuccess,
		AActor* NewTarget, const float Duration = 1.0f, const EViewTargetBlendFunction BlendFunc = VTBlend_Linear,
		const float BlendExp = 0.0f, const bool bLockOutgoing = false, const int32 PlayerIdx = 0);
};
