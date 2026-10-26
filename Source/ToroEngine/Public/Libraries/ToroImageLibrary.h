// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "UE5Coro.h"
#include "DataTypes/ToroImageData.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ToroImageLibrary.generated.h"

/**
 * Blueprint utilities for widget rendering, screenshots, CPU image data, transient textures, and PNG file export.
 * Rendering, capture, and texture operations run on the game thread; async exports offload only file writing.
 */
UCLASS(NotBlueprintable, NotBlueprintType)
class TOROENGINE_API UToroImageLibrary final : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/** BGRA8 pixel format required for texture extraction and used for transient texture creation. */
	static constexpr EPixelFormat SupportedFormat = PF_B8G8R8A8;

	/**
	 * Requests a screenshot of the global game viewport and waits for captured pixels or request processing.
	 * Clears Image before validation. Completion may leave it empty if no supported pixel callback arrives,
	 * including when screenshot delegates are disabled or capture uses HDR pixels; use IsValidImageData() to check.
	 * Uses global screenshot state and delegates; concurrent requests are not isolated from one another.
	 * @param Image Receives captured dimensions and pixels, or remains empty on rejection or capture failure.
	 * @param ResolutionScale Finite positive viewport-size multiplier. Non-unit scales use high-resolution capture,
	 * require at least one pixel per axis, and must fit the engine's maximum texture dimensions.
	 * @param bIncludeUI Whether to capture UI; supported only when ResolutionScale is exactly 1.
	 */
	UFUNCTION(BlueprintCallable, Category = ImageUtils, meta = (Latent, LatentInfo = LatentInfo))
	static FVoidCoroutine RequestScreenshot(FLatentActionInfo LatentInfo, FToroImageData& Image,
		float ResolutionScale = 1.0f, bool bIncludeUI = false);

	/**
	 * Tests whether both image dimensions are positive and the pixel count equals their product.
	 * @param InData Image data to validate.
	 */
	UFUNCTION(BlueprintPure, Category = ImageUtils)
	[[nodiscard]] static bool IsValidImageData(const FToroImageData& InData);

	/**
	 * Resets image dimensions to zero and removes all pixels.
	 * @param InData Image data to clear, leaving an invalid empty image.
	 */
	UFUNCTION(BlueprintCallable, Category = ImageUtils)
	static void ClearImageData(UPARAM(ref) FToroImageData& InData);

	/**
	 * Submits a widget draw to the render target using its dimensions and the widget's cached geometry scale.
	 * Uses a zero draw delta time and returns without waiting for GPU completion; invalid inputs are ignored.
	 * @param Target Render target receiving the widget draw.
	 * @param UserWidget Widget to render.
	 * @param bGammaCorrection Whether the widget renderer applies gamma correction.
	 * @param bInClearTarget Whether the renderer clears the target before drawing.
	 */
	UFUNCTION(BlueprintCallable, Category = ImageUtils)
	static void DrawWidgetToRenderTarget(UTextureRenderTarget2D* Target, UUserWidget* UserWidget,
		bool bGammaCorrection = true, bool bInClearTarget = false);

	/**
	 * Copies the first platform-data mip from a texture into CPU image data.
	 * Requires PF_B8G8R8A8 and accessible bulk pixel data; does not decompress unsupported texture formats.
	 * @param OutData Replaced with the first mip's dimensions and pixels; cleared on failure.
	 * @param Target Texture whose first platform-data mip is read.
	 * @return True if the pixels were copied; false for invalid input, unsupported format, or unavailable or invalid bulk data.
	 */
	UFUNCTION(BlueprintCallable, Category = ImageUtils)
	[[nodiscard]] static bool GetDataFromTexture(FToroImageData& OutData, const UTexture2D* Target);

	/**
	 * Reads render-target pixels into CPU FColor image data on the game thread.
	 * Readback can block while rendering completes and uses the engine's default ReadPixels conversion.
	 * @param OutData Replaced with the target's dimensions and pixels; cleared on failure.
	 * @param Target Render target to read.
	 * @param bInvertAlpha Whether to replace each alpha byte with 255 minus its value.
	 * @return True if readback produces valid image data; false for invalid input, unavailable resources, or readback failure.
	 */
	UFUNCTION(BlueprintCallable, Category = ImageUtils)
	[[nodiscard]] static bool GetDataFromRenderTarget(FToroImageData& OutData, UTextureRenderTarget2D* Target, bool bInvertAlpha = false);

	/**
	 * Creates a transient PF_B8G8R8A8 texture from valid image data, with sRGB enabled and default compression settings.
	 * The caller must retain a UObject reference to keep the returned texture alive.
	 * @param InData Positive image dimensions and exactly one FColor per pixel.
	 * @return The new transient texture, or nullptr for invalid data or texture creation or pixel-storage failure.
	 */
	UFUNCTION(BlueprintCallable, Category = ImageUtils)
	[[nodiscard]] static UTexture2D* CreateTextureFromData(const FToroImageData& InData);

	/**
	 * Extracts a texture's first platform-data mip and saves it as PNG, regardless of the filename extension.
	 * Requires PF_B8G8R8A8 and accessible CPU bulk pixel data, as for GetDataFromTexture().
	 * @param bSuccess Receives whether extraction, PNG encoding, and file writing succeeded.
	 * @param Target Texture to extract and save.
	 * @param FilePath Destination filename passed to native file writing; must be valid and writable.
	 * @param bAsync Whether to write on a worker thread and resume on the game thread.
	 * Extraction and PNG compression still run on the calling game thread; false also performs file writing there.
	 */
	UFUNCTION(BlueprintCallable, Category = ImageUtils, meta = (Latent, LatentInfo = LatentInfo))
	static FVoidCoroutine SaveTextureToFile(FLatentActionInfo LatentInfo, bool& bSuccess,
		const UTexture2D* Target, const FString& FilePath, bool bAsync = true);

	/**
	 * Reads a render target and saves its pixels as PNG, regardless of the filename extension.
	 * Uses GetDataFromRenderTarget() and its default engine pixel conversion; readback may block.
	 * @param bSuccess Receives whether readback, PNG encoding, and file writing succeeded.
	 * @param Target Render target to read and save.
	 * @param FilePath Destination filename passed to native file writing; must be valid and writable.
	 * @param bInvertAlpha Whether to replace each alpha byte with 255 minus its value before encoding.
	 * @param bAsync Whether to write on a worker thread and resume on the game thread.
	 * Readback and PNG compression still run on the calling game thread; false also performs file writing there.
	 */
	UFUNCTION(BlueprintCallable, Category = ImageUtils, meta = (Latent, LatentInfo = LatentInfo))
	static FVoidCoroutine SaveRenderTargetToFile(FLatentActionInfo LatentInfo, bool& bSuccess,
		UTextureRenderTarget2D* Target, const FString& FilePath, bool bInvertAlpha, bool bAsync = true);

	/**
	 * Encodes valid image data as PNG and writes it to the destination, regardless of the filename extension.
	 * @param bSuccess Receives whether path validation, PNG encoding, and file writing succeeded.
	 * @param InData Image data with positive dimensions and exactly one FColor per pixel.
	 * @param FilePath Destination filename; copied before the worker-thread switch so the async write owns its path.
	 * @param bAsync Whether to write on a worker thread and resume on the game thread.
	 * PNG compression still runs on the calling game thread; false also performs file writing there.
	 */
	UFUNCTION(BlueprintCallable, Category = ImageUtils, meta = (Latent, LatentInfo = LatentInfo))
	static FVoidCoroutine SaveImageDataToFile(FLatentActionInfo LatentInfo, bool& bSuccess,
		const FToroImageData& InData, const FString& FilePath, bool bAsync = true);
};
