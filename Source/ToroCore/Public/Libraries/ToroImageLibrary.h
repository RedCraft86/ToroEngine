// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#pragma once

#include "UE5Coro.h"
#include "PixelFormat.h"
#include "DataTypes/ToroImageData.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ToroImageLibrary.generated.h"

class UTexture2D;
class UUserWidget;
class UTextureRenderTarget2D;

/**
 * Widget rendering, BGRA8 image readback, texture creation, PNG saving, and screenshots.
 * Rendering, readback, texture creation, and screenshot requests require the game thread.
 */
UCLASS(NotBlueprintable, NotBlueprintType)
class TOROCORE_API UToroImageLibrary final : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/** Platform format accepted for texture readback and used for transient texture creation. */
	static constexpr EPixelFormat SupportedFormat = PF_B8G8R8A8;

	/**
	 * Draws the widget at the render target's dimensions; invalid inputs are ignored.
	 * @param Target Render target to draw into.
	 * @param UserWidget Widget whose current Slate representation is drawn.
	 * @param bGammaCorrection Enable gamma correction in the widget renderer.
	 * @param bInClearTarget Clear the target before drawing.
	 */
	UFUNCTION(BlueprintCallable, Category = ImageUtils)
	static void DrawWidgetToRenderTarget(UTextureRenderTarget2D* Target, UUserWidget* UserWidget,
		bool bGammaCorrection = true, bool bInClearTarget = false);

	/**
	 * Copies the first mip of a texture with BGRA8 platform data into image pixels.
	 * CPU bulk data must be available and large enough; streamed or cooked data may be unavailable.
	 * @param OutData Replaced with image data on success; empty on failure.
	 * @param Target Texture to read.
	 * @return True for valid dimensions and copied pixels; false for invalid, unsupported, or unavailable data.
	 */
	UFUNCTION(BlueprintCallable, Category = ImageUtils)
	[[nodiscard]] static bool GetDataFromTexture(FToroImageData& OutData, const UTexture2D* Target);

	/**
	 * Reads render-target pixels synchronously on the game thread.
	 * @param OutData Replaced with dimension-matched pixels on success; empty on failure.
	 * @param Target Render target to read.
	 * @param bInvertAlpha Replace each alpha byte with 255 minus its value.
	 * @return True for a successful, dimension-matched readback; false otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = ImageUtils)
	[[nodiscard]] static bool GetDataFromRenderTarget(FToroImageData& OutData, UTextureRenderTarget2D* Target, bool bInvertAlpha = false);

	/**
	 * Creates a transient, sRGB BGRA8 texture from valid image data.
	 * @param InData Dimensions and row-major pixels to copy.
	 * @return New texture, or nullptr for invalid data or creation failure. Keep a UObject reference to retain it.
	 */
	UFUNCTION(BlueprintCallable, Category = ImageUtils)
	[[nodiscard]] static UTexture2D* CreateTextureFromData(const FToroImageData& InData);

	/**
	 * Reads a texture and saves its pixels as PNG regardless of the filename extension.
	 * @param bSuccess Receives whether readback, encoding, and file writing succeeded.
	 * @param Target Texture with readable BGRA8 platform data.
	 * @param FilePath Destination path; parent directories are not created automatically.
	 * @param bAsync Perform file writing on a worker and resume on the game thread; PNG encoding precedes that move.
	 */
	UFUNCTION(BlueprintCallable, Category = ImageUtils, meta = (Latent, LatentInfo = LatentInfo))
	static FVoidCoroutine SaveTextureToFile(FLatentActionInfo LatentInfo, bool& bSuccess,
		const UTexture2D* Target, const FString& FilePath, bool bAsync = true);

	/**
	 * Reads a render target and saves its pixels as PNG regardless of the filename extension.
	 * @param bSuccess Receives whether readback, encoding, and file writing succeeded.
	 * @param Target Render target to read on the game thread.
	 * @param FilePath Destination path; parent directories are not created automatically.
	 * @param bInvertAlpha Invert alpha bytes before encoding.
	 * @param bAsync Perform file writing on a worker and resume on the game thread; PNG encoding precedes that move.
	 */
	UFUNCTION(BlueprintCallable, Category = ImageUtils, meta = (Latent, LatentInfo = LatentInfo))
	static FVoidCoroutine SaveRenderTargetToFile(FLatentActionInfo LatentInfo, bool& bSuccess,
		UTextureRenderTarget2D* Target, const FString& FilePath, bool bInvertAlpha, bool bAsync = true);

	/**
	 * Encodes image data as PNG and writes it to the supplied path regardless of extension.
	 * @param bSuccess Receives whether validation, encoding, and file writing succeeded.
	 * @param InData Image to encode; dimensions and pixel count must be consistent.
	 * @param FilePath Destination path; parent directories are not created automatically.
	 * @param bAsync Move file I/O to a worker and resume on the game thread; encoding occurs before the move.
	 */
	UFUNCTION(BlueprintCallable, Category = ImageUtils, meta = (Latent, LatentInfo = LatentInfo))
	static FVoidCoroutine SaveImageDataToFile(FLatentActionInfo LatentInfo, bool& bSuccess,
		const FToroImageData& InData, const FString& FilePath, bool bAsync = true);

	/**
	 * Requests an LDR game-viewport screenshot and waits for capture or request processing.
	 * The global screenshot mechanism does not isolate concurrent requests; the next shared capture may satisfy multiple callers.
	 * HDR or disabled capture delegates can finish processing without producing LDR pixels.
	 * No timeout is imposed: processing requires the viewport to continue rendering.
	 * @param Image Replaced by captured pixels; empty for invalid requests or processing without LDR capture.
	 * @param ResolutionScale Finite positive scale; values other than one use high-resolution capture.
	 * @param bIncludeUI Include Slate UI; supported only with ResolutionScale equal to one.
	 */
	UFUNCTION(BlueprintCallable, Category = ImageUtils, meta = (Latent, LatentInfo = LatentInfo))
	static FVoidCoroutine RequestScreenshot(FLatentActionInfo LatentInfo, FToroImageData& Image,
		float ResolutionScale = 1.0f, bool bIncludeUI = false);
};
