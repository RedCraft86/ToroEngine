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
	UUserWidget* UserWidget, const bool bGammaCorrection, const bool bInClearTarget)
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
		UE_LOG(LogToroCore, Warning, TEXT("UToroImageLibrary::GetDataFromTexture - Invalid PlatformData or no Mips"));
		return false;
	}

	if (PlatformData->PixelFormat != SupportedFormat)
	{
		UE_LOG(LogToroCore, Warning, TEXT("UToroImageLibrary::GetDataFromTexture - Unsupported PixelFormat"));
		return false;
	}

	const auto& Mip = PlatformData->Mips[0];
	const int64 PixelCount = static_cast<int64>(Mip.SizeX) * static_cast<int64>(Mip.SizeY);
	if (Mip.SizeX <= 0 || Mip.SizeY <= 0 || PixelCount > MAX_int32
		|| Mip.BulkData.GetBulkDataSize() < PixelCount * static_cast<int64>(sizeof(FColor)))
	{
		UE_LOG(LogToroCore, Warning, TEXT("UToroImageLibrary::GetDataFromTexture - Size check failed"));
		return false;
	}

	const void* TexData = Mip.BulkData.LockReadOnly();
	ON_SCOPE_EXIT { Mip.BulkData.Unlock(); };
	if (TexData)
	{
		OutData.Size = FIntPoint(Mip.SizeX, Mip.SizeY);
		OutData.Pixels.SetNumUninitialized(static_cast<int32>(PixelCount));
		FMemory::Memcpy(OutData.Pixels.GetData(), TexData, static_cast<SIZE_T>(PixelCount) * sizeof(FColor));
		return true;
	}

	UE_LOG(LogToroCore, Warning, TEXT("UToroImageLibrary::GetDataFromTexture - Failed to lock BulkData"));
	OutData.Empty();
	return false;
}

bool UToroImageLibrary::GetDataFromRenderTarget(FToroImageData& OutData, UTextureRenderTarget2D* Target, const bool bInvertAlpha)
{
	OutData.Empty();
	if (!IsValid(Target))
	{
		return false;
	}

	const int64 PixelCount = static_cast<int64>(Target->SizeX) * static_cast<int64>(Target->SizeY);
	if (Target->SizeX <= 0 || Target->SizeY <= 0 || PixelCount > TNumericLimits<int32>::Max())
	{
		UE_LOG(LogToroCore, Warning, TEXT("UToroImageLibrary::GetDataFromRenderTarget - Pixel count goes beyond Int32 limit."));
		return false;
	}

	if (FTextureRenderTargetResource* Resource = Target->GameThread_GetRenderTargetResource())
	{
		if (!Resource->ReadPixels(OutData.Pixels))
		{
			UE_LOG(LogToroCore, Warning, TEXT("UToroImageLibrary::GetDataFromRenderTarget - Failed to read pixels."));
			OutData.Empty();
			return false;
		}

		OutData.Size = FIntPoint(Target->SizeX, Target->SizeY);
		if (!OutData.IsValid())
		{
			UE_LOG(LogToroCore, Warning, TEXT("UToroImageLibrary::GetDataFromRenderTarget - Pixel data and size mismatch."));
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

	UE_LOG(LogToroCore, Warning, TEXT("UToroImageLibrary::GetDataFromRenderTarget - Failed to obtain resource."));
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
		UE_LOG(LogToroCore, Warning, TEXT("UToroImageLibrary::CreateTextureFromData - Failed to create texture"));
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
		UE_LOG(LogToroCore, Warning, TEXT("UToroImageLibrary::CreateTextureFromData - Invalid PlatformData or Mips"));
		Image->MarkAsGarbage();
		return nullptr;
	}

	FTexture2DMipMap& Mip = PlatformData->Mips[0];
	void* TextureData = Mip.BulkData.Lock(LOCK_READ_WRITE);
	if (!TextureData)
	{
		UE_LOG(LogToroCore, Warning, TEXT("UToroImageLibrary::CreateTextureFromData - Failed to lock BulkData"));
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
	const UTexture2D* Target, const FString& FilePath, const bool bAsync)
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
	UTextureRenderTarget2D* Target, const FString& FilePath, const bool bInvertAlpha, const bool bAsync)
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
	const FToroImageData& InData, const FString& FilePath, const bool bAsync)
{
	if (!FPaths::ValidatePath(FilePath))
	{
		UE_LOG(LogToroCore, Warning, TEXT("UToroImageLibrary::SaveImageDataToFile - Invalid export path"));
		bSuccess = false;
		co_return;
	}

	TArray64<uint8> PNGImage;
	if (!InData.CompressPNG(PNGImage))
	{
		UE_LOG(LogToroCore, Warning, TEXT("UToroImageLibrary::SaveImageDataToFile - Failed to compress PNG"));
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
	FToroImageData& Image, const float ResolutionScale, const bool bIncludeUI)
{
	Image.Empty();
	if (!FMath::IsFinite(ResolutionScale) || ResolutionScale <= 0.0f)
	{
		UE_LOG(LogToroCore, Warning, TEXT("UToroImageLibrary::RequestScreenshot - Called with invalid resolution scale."))
		co_return;
	}

	FViewport* VP = (GEngine && GEngine->GameViewport) ? GEngine->GameViewport->Viewport : nullptr;
	if (!VP || VP->GetSizeXY().X <= 0 || VP->GetSizeXY().Y <= 0)
	{
		UE_LOG(LogToroCore, Warning, TEXT("UToroImageLibrary::RequestScreenshot - No game viewport available."));
		co_return;
	}

	if (ResolutionScale != 1.0f && bIncludeUI)
	{
		UE_LOG(LogToroCore, Warning, TEXT("UToroImageLibrary::RequestScreenshot - Abnormal resolution scales do not support UI capture."));
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
			UE_LOG(LogToroCore, Warning, TEXT("UToroImageLibrary::RequestScreenshot - Resolution scale too low (<1 pixel on one or both axis)."))
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
