// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#include "Libraries/ToroImageLibrary.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/GameViewportClient.h"
#include "Blueprint/UserWidget.h"
#include "Slate/WidgetRenderer.h"
#include "HighResScreenshot.h"
#include "Engine/Texture2D.h"
#include "Engine/Engine.h"
#include "UnrealClient.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "Misc/ScopeExit.h"
#include "ToroCore.h"

namespace
{
	/** Copies the next LDR screenshot broadcast into the supplied image. */
	UE5Coro::TCoroutine<> WaitForScreenshotCapture(FToroImageData& Image)
	{
		auto&& [Width, Height, Pixels] = co_await UGameViewportClient::OnScreenshotCaptured();
		Image.Size = FIntPoint(Width, Height);
		Image.Pixels = Pixels;
	}

	/** Finishes when screenshot processing ends, including paths without an LDR capture. */
	UE5Coro::TCoroutine<> WaitForScreenshotProcessed()
	{
		co_await FScreenshotRequest::OnScreenshotRequestProcessed();
	}
}

void UToroImageLibrary::DrawWidgetToRenderTarget(UTextureRenderTarget2D* Target,
	UUserWidget* UserWidget, bool bGammaCorrection, bool bInClearTarget)
{
	if (IsValid(Target) && IsValid(UserWidget))
	{
		const TSharedRef<SWidget> Widget = UserWidget->TakeWidget();

		FWidgetRenderer* Renderer = new FWidgetRenderer(bGammaCorrection, bInClearTarget);
		Renderer->DrawWidget(Target, Widget,
			Widget->GetCachedGeometry().Scale,
			FVector2D(Target->SizeX, Target->SizeY),
			0.0f
		);

		BeginCleanup(Renderer);
	}
}

bool UToroImageLibrary::GetDataFromTexture(FToroImageData& OutData, const UTexture2D* Target)
{
	OutData.Empty();
	if (!IsValid(Target))
	{
		return false;
	}

	const FTexturePlatformData* PlatformData = Target->GetPlatformData();
	if (!PlatformData || PlatformData->Mips.IsEmpty())
	{
		FFrame::KismetExecutionMessage(TEXT("Invalid PlatformData or no Mips."), ELogVerbosity::Error);
		return false;
	}

	if (PlatformData->PixelFormat != SupportedFormat)
	{
		FFrame::KismetExecutionMessage(TEXT("Unsupported pixel format."), ELogVerbosity::Error);
		return false;
	}

	const auto& Mip = PlatformData->Mips[0];
	const int64 PixelCount = static_cast<int64>(Mip.SizeX) * static_cast<int64>(Mip.SizeY);
	if (Mip.SizeX <= 0 || Mip.SizeY <= 0 || PixelCount > MAX_int32
		|| Mip.BulkData.GetBulkDataSize() < PixelCount * static_cast<int64>(sizeof(FColor)))
	{
		FFrame::KismetExecutionMessage(TEXT("Invalid pixel count."), ELogVerbosity::Error);
		return false;
	}

	const void* TexData = Mip.BulkData.LockReadOnly();
	ON_SCOPE_EXIT { Mip.BulkData.Unlock(); };
	if (!TexData)
	{
		FFrame::KismetExecutionMessage(TEXT("Failed to lock BulkData."), ELogVerbosity::Error);
		OutData.Empty();
		return false;
	}

	OutData.Size = FIntPoint(Mip.SizeX, Mip.SizeY);
	OutData.Pixels.SetNumUninitialized(static_cast<int32>(PixelCount));
	FMemory::Memcpy(OutData.Pixels.GetData(), TexData, static_cast<SIZE_T>(PixelCount) * sizeof(FColor));
	return true;
}

bool UToroImageLibrary::GetDataFromRenderTarget(FToroImageData& OutData, UTextureRenderTarget2D* Target, bool bInvertAlpha)
{
	OutData.Empty();
	if (!IsValid(Target))
	{
		return false;
	}

	const int64 PixelCount = static_cast<int64>(Target->SizeX) * static_cast<int64>(Target->SizeY);
	if (Target->SizeX <= 0 || Target->SizeY <= 0 || PixelCount > TNumericLimits<int32>::Max())
	{
		
		FFrame::KismetExecutionMessage(TEXT("Invalid pixel count."), ELogVerbosity::Error);
		return false;
	}

	if (FTextureRenderTargetResource* Resource = Target->GameThread_GetRenderTargetResource())
	{
		if (!Resource->ReadPixels(OutData.Pixels))
		{
			FFrame::KismetExecutionMessage(TEXT("Failed to read pixels."), ELogVerbosity::Error);
			OutData.Empty();
			return false;
		}

		OutData.Size = FIntPoint(Target->SizeX, Target->SizeY);
		if (!OutData.IsValid())
		{
			
			FFrame::KismetExecutionMessage(TEXT("Pixel count and data size mismatch."), ELogVerbosity::Error);
			OutData.Empty();
			return false;
		}

		if (bInvertAlpha)
		{
			for (FColor& Pixel : OutData.Pixels)
			{
				Pixel.A = 255 - Pixel.A;
			}
		}

		return true;
	}

	FFrame::KismetExecutionMessage(TEXT("Failed to obtain resource."), ELogVerbosity::Error);
	return false;
}

UTexture2D* UToroImageLibrary::CreateTextureFromData(const FToroImageData& InData)
{
	if (!InData.IsValid())
	{
		return nullptr;
	}

	UTexture2D* Image = UTexture2D::CreateTransient(InData.Size.X, InData.Size.Y, SupportedFormat);
	if (!IsValid(Image))
	{
		FFrame::KismetExecutionMessage(TEXT("Failed to create texture."), ELogVerbosity::Error);
		return nullptr;
	}

#if WITH_EDITORONLY_DATA
	Image->MipGenSettings = TMGS_NoMipmaps;
#endif
	Image->CompressionSettings = TC_Default;
	Image->SRGB = true;

	FTexturePlatformData* PlatformData = Image->GetPlatformData();
	if (!PlatformData || PlatformData->Mips.Num() == 0)
	{
		FFrame::KismetExecutionMessage(TEXT("Invalid PlatformData or no Mips."), ELogVerbosity::Error);
		Image->MarkAsGarbage();
		return nullptr;
	}

	FTexture2DMipMap& Mip = PlatformData->Mips[0];
	void* TextureData = Mip.BulkData.Lock(LOCK_READ_WRITE);
	if (!TextureData)
	{
		FFrame::KismetExecutionMessage(TEXT("Failed to lock BulkData."), ELogVerbosity::Error);
		Mip.BulkData.Unlock();
		Image->MarkAsGarbage();
		return nullptr;
	}

	FMemory::Memcpy(TextureData, InData.Pixels.GetData(), sizeof(FColor) * static_cast<SIZE_T>(InData.Pixels.Num()));
	Mip.BulkData.Unlock();

	Image->UpdateResource();

	return Image;
}

FVoidCoroutine UToroImageLibrary::SaveTextureToFile(FLatentActionInfo LatentInfo, bool& bSuccess,
	const UTexture2D* Target, const FString& FilePath, bool bAsync)
{
	FToroImageData Data;
	if (!GetDataFromTexture(Data, Target))
	{
		bSuccess = false;
		co_return;
	}

	co_await SaveImageDataToFile(FLatentActionInfo(), bSuccess, Data, FilePath, bAsync);
	co_return;
}

FVoidCoroutine UToroImageLibrary::SaveRenderTargetToFile(FLatentActionInfo LatentInfo, bool& bSuccess,
	UTextureRenderTarget2D* Target, const FString& FilePath, bool bInvertAlpha, bool bAsync)
{
	FToroImageData Data;
	if (!GetDataFromRenderTarget(Data, Target, bInvertAlpha))
	{
		bSuccess = false;
		co_return;
	}

	co_await SaveImageDataToFile(FLatentActionInfo(), bSuccess, Data, FilePath, bAsync);
	co_return;
}

FVoidCoroutine UToroImageLibrary::SaveImageDataToFile(FLatentActionInfo LatentInfo, bool& bSuccess,
	const FToroImageData& InData, const FString& FilePath, bool bAsync)
{
	if (!FPaths::ValidatePath(FilePath))
	{
		FFrame::KismetExecutionMessage(TEXT("Invalid export path."), ELogVerbosity::Error);
		bSuccess = false;
		co_return;
	}

	TArray64<uint8> PNGImage;
	if (!InData.CompressPNG(PNGImage))
	{
		FFrame::KismetExecutionMessage(TEXT("Failed to compress PNG."), ELogVerbosity::Error);
		bSuccess = false;
		co_return;
	}

	if (bAsync)
	{
		co_await UE5Coro::Async::MoveToTask();
		const bool bResult = FFileHelper::SaveArrayToFile(PNGImage, *FilePath);
		co_await UE5Coro::Async::MoveToGameThread();
		bSuccess = bResult;
		co_return;
	}

	bSuccess = FFileHelper::SaveArrayToFile(PNGImage, *FilePath);
	co_return;
}

FVoidCoroutine UToroImageLibrary::RequestScreenshot(FLatentActionInfo LatentInfo,
	FToroImageData& Image, float ResolutionScale, bool bIncludeUI)
{
	Image.Empty();
	if (!FMath::IsFinite(ResolutionScale) || ResolutionScale <= 0.0f)
	{
		FFrame::KismetExecutionMessage(TEXT("Invalid resolution scale."), ELogVerbosity::Error);
		co_return;
	}

	FViewport* VP = (GEngine && GEngine->GameViewport) ? GEngine->GameViewport->Viewport : nullptr;
	if (!VP || VP->GetSizeXY().X <= 0 || VP->GetSizeXY().Y <= 0)
	{
		FFrame::KismetExecutionMessage(TEXT("No game viewport client found."), ELogVerbosity::Error);
		co_return;
	}

	if (ResolutionScale != 1.0f && bIncludeUI)
	{
		FFrame::KismetExecutionMessage(TEXT("Abnormal resolution scales do not support UI capture."), ELogVerbosity::Warning);
		co_return;
	}

	auto CapturedOrProcessed = UE5Coro::Race(
			WaitForScreenshotCapture(Image),
			WaitForScreenshotProcessed()
	);

	if (ResolutionScale == 1.0f)
	{
		FScreenshotRequest::RequestScreenshot(bIncludeUI, true);
	}
	else
	{
		const FIntPoint Size = VP->GetSizeXY();
		if ((Size.X * ResolutionScale) < 1.0f || (Size.Y * ResolutionScale) < 1.0f)
		{
			FFrame::KismetExecutionMessage(TEXT("Resolution scale too low. <1 pixel on one or both axis."), ELogVerbosity::Error);
			co_return;
		}

		if (!GetHighResScreenshotConfig().SetResolution(Size.X, Size.Y, ResolutionScale)
			|| !VP->TakeHighResScreenShot())
		{
			co_return;
		}
	}

	co_await CapturedOrProcessed;
}
