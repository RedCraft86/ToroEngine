// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "UE5Coro.h"
#include "CoreMinimal.h"
#include "Camera/PlayerCameraManager.h"
#include "Engine/LatentActionManager.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ToroCameraLibrary.generated.h"

class AActor;
class UCameraComponent;
class USceneCaptureComponent2D;

/**
 * Blueprint helpers for bounds-based frustum tests, player views, camera fades, and view-target transitions.
 */
UCLASS(NotBlueprintable, NotBlueprintType)
class TOROENGINE_API UToroCameraLibrary final : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/**
	 * Tests actor world-space axis-aligned bounds against the camera component's frustum.
	 * Includes child actor bounds and noncolliding components; does not test occlusion or rendered visibility.
	 * @param Target Camera component supplying the view.
	 * @param TestActor Actor whose bounds are tested.
	 * @return True for a bounds intersection; false for invalid inputs or no intersection.
	 */
	UFUNCTION(BlueprintPure, Category = Camera, meta = (DefaultToSelf = Target))
	[[nodiscard]] static bool IsActorInCameraFrustum(UCameraComponent* Target, const AActor* TestActor);

	/**
	 * Tests actor bounds against the scene capture's frustum, honoring its enabled custom projection matrix.
	 * Includes child actor bounds and noncolliding components; does not test occlusion, capture filtering, or rendered visibility.
	 * @param Target Scene capture supplying the view and optional custom projection.
	 * @param TestActor Actor whose world-space axis-aligned bounds are tested.
	 * @return True for a bounds intersection; false for invalid inputs or no intersection.
	 */
	UFUNCTION(BlueprintPure, Category = "Rendering|SceneCapture", DisplayName = "Is Actor in Scene Capture 2D Frustum", meta = (DefaultToSelf = Target))
	[[nodiscard]] static bool IsActorInSceneCapture2DFrustum(USceneCaptureComponent2D* Target, const AActor* TestActor);

	/**
	 * Tests actor bounds against a local player's viewport frustum in the actor's world.
	 * Includes child actor bounds and noncolliding components; does not test occlusion or rendered visibility.
	 * @param TestActor Actor whose world-space axis-aligned bounds are tested.
	 * @param PlayerIdx Player controller index used to resolve the local player.
	 * @return True for a bounds intersection; false for an invalid actor, unavailable local viewport or projection, or no intersection.
	 */
	UFUNCTION(BlueprintPure, Category = "Game|Player|View", meta = (DefaultToSelf = TestActor))
	[[nodiscard]] static bool IsActorInViewFrustum(const AActor* TestActor, int32 PlayerIdx = 0);

	/**
	 * Gets the selected player camera's location and rotation with unit scale.
	 * In editor builds outside game mode, uses the active registered editor viewport instead.
	 * Successful results are cached per frame by camera manager or editor viewport; failures are not cached.
	 * @param PlayerIdx Player controller index; ignored when using an editor viewport.
	 * @return The view transform, or identity when the required camera or editor viewport is unavailable.
	 */
	UFUNCTION(BlueprintCallable, Category = "Game|Player|View", meta = (WorldContext = ContextObject))
	[[nodiscard]] static FTransform GetViewTransform(const UObject* ContextObject, int32 PlayerIdx = 0);

	/**
	 * Stops camera fading and any associated audio fade for the selected player.
	 * @param PlayerIdx Player controller index used to resolve the camera manager.
	 * @return True if a camera manager was found and the stop request was issued, even if no fade was active.
	 */
	UFUNCTION(BlueprintCallable, Category = "Game|Player|View", meta = (WorldContext = ContextObject))
	static bool StopCameraFade(const UObject* ContextObject, int32 PlayerIdx = 0);

	/**
	 * Sets a manual camera fade that remains until changed or stopped.
	 * @param Color Fade overlay color.
	 * @param Alpha Fade opacity, conventionally zero for transparent and one for opaque; passed through without clamping.
	 * @param bFadeAudio Whether audio volume should follow the fade amount.
	 * @param PlayerIdx Player controller index used to resolve the camera manager.
	 * @return True if a camera manager was found and the manual fade was applied.
	 */
	UFUNCTION(BlueprintCallable, Category = "Game|Player|View", meta = (WorldContext = ContextObject))
	static bool SetCameraFade(const UObject* ContextObject, FLinearColor Color = FLinearColor::Black,
		float Alpha = 1.0f, bool bFadeAudio = true, int32 PlayerIdx = 0);

	/**
	 * Starts a camera fade and waits for the clamped duration.
	 * The wait does not track interruption or replacement of the fade and does not verify its final rendered state.
	 * @param bSuccess True after issuing the request and completing its wait; false for nonfinite duration or a missing camera manager.
	 * @param Color Fade overlay color.
	 * @param Duration Finite seconds to fade; negative values are silently clamped to zero.
	 *        Nonfinite values report an error through Toro logging without changing the fade.
	 * @param FromAlpha Starting opacity, conventionally zero to one; passed through without clamping.
	 * @param ToAlpha Ending opacity, conventionally zero to one; passed through without clamping.
	 * @param bFadeAudio Whether audio volume should follow the fade amount.
	 * @param bHoldAtEnd Whether to retain the ending fade. At a clamped duration of zero, applies ToAlpha immediately when true, otherwise stops fading.
	 * @param PlayerIdx Player controller index used to resolve the camera manager.
	 */
	UFUNCTION(BlueprintCallable, Category = "Game|Player|View", meta = (Latent, LatentInfo = LatentInfo,
		WorldContext = ContextObject, AdvancedDisplay = "FromAlpha, ToAlpha, bFadeAudio, bHoldAtEnd"))
	static FVoidCoroutine StartCameraFade(FLatentActionInfo LatentInfo, const UObject* ContextObject, bool& bSuccess,
		FLinearColor Color = FLinearColor::Black, float Duration = 1.0f, float FromAlpha = 0.0f,
		float ToAlpha = 1.0f, bool bFadeAudio = true, bool bHoldAtEnd = true, int32 PlayerIdx = 0);

	/**
	 * Gets the selected player controller's current view target.
	 * @param PlayerIdx Player controller index.
	 * @return The current view target, or nullptr when no valid player controller is available.
	 */
	UFUNCTION(BlueprintCallable, Category = "Game|Player|View", meta = (WorldContext = ContextObject))
	[[nodiscard]] static AActor* GetPlayerViewTarget(const UObject* ContextObject, int32 PlayerIdx = 0);

	/**
	 * Requests an immediate view-target switch for the selected player controller.
	 * @param NewTarget Actor to view; nullptr uses the engine's fallback to the player controller.
	 * @param PlayerIdx Player controller index.
	 * @return True if a valid player controller was found and the request was issued.
	 */
	UFUNCTION(BlueprintCallable, Category = "Game|Player|View", meta = (WorldContext = ContextObject))
	static bool SetPlayerViewTarget(const UObject* ContextObject, AActor* NewTarget, int32 PlayerIdx = 0);

	/**
	 * Requests a view-target blend and waits for the clamped duration.
	 * The wait does not track interruption or replacement of the blend and does not verify the final view target.
	 * @param bSuccess True after issuing the request and completing its wait; false for nonfinite duration or a missing controller.
	 * @param NewTarget Actor to view; nullptr uses the engine's fallback to the player controller.
	 * @param Duration Finite seconds to blend; negative values are silently clamped to zero for an immediate switch.
	 *        Nonfinite values report an error through Toro logging without changing the view target.
	 * @param BlendFunc Engine interpolation function used for the blend.
	 * @param BlendExp Exponent used by blend functions that support it.
	 * @param bLockOutgoing Whether to freeze the outgoing view at its last-frame position for the blend.
	 * @param PlayerIdx Player controller index.
	 */
	UFUNCTION(BlueprintCallable, Category = "Game|Player|View", meta = (Latent, LatentInfo = LatentInfo,
		WorldContext = ContextObject, DefaultToSelf = NewTarget, AdvancedDisplay = "BlendFunc, BlendExp, bLockOutgoing"))
	static FVoidCoroutine BlendPlayerViewTarget(FLatentActionInfo LatentInfo, const UObject* ContextObject, bool& bSuccess,
		AActor* NewTarget, float Duration = 1.0f, EViewTargetBlendFunction BlendFunc = VTBlend_Linear,
		float BlendExp = 0.0f, bool bLockOutgoing = false, int32 PlayerIdx = 0);
};
