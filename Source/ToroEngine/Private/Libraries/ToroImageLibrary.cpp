// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "Libraries/ToroImageLibrary.h"
#include "Utilities/ToroLogging.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/GameViewportClient.h"
#include "Engine/Texture2D.h"
#include "Engine/Engine.h"
#include "Slate/WidgetRenderer.h"
#include "Blueprint/UserWidget.h"
#include "HighResScreenshot.h"
#include "Widgets/SWidget.h"
#include "Misc/ScopeExit.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "TextureResource.h"
#include "RenderDeferredCleanup.h"
#include "UnrealClient.h"

namespace
{
	UE5Coro::TCoroutine<> AwaitScreenshotCapture(FToroImageData& Image)
	{
		auto&& [Width, Height, Pixels] = co_await UGameViewportClient::OnScreenshotCaptured();
		Image.Size = FIntPoint(Width, Height);
		Image.Pixels = Pixels;
	}

	UE5Coro::TCoroutine<> AwaitScreenshotProcessed()
	{
		co_await FScreenshotRequest::OnScreenshotRequestProcessed();
	}
}

FVoidCoroutine UToroImageLibrary::RequestScreenshot(FLatentActionInfo LatentInfo,
	FToroImageData& Image, float ResolutionScale, bool bIncludeUI)
{
	Image.Clear();
	if (!FMath::IsFinite(ResolutionScale) || ResolutionScale <= 0.0f)
	{
		TORO_REPORT_ERROR(TEXT("Invalid resolution scale."));
		co_return;
	}

	FViewport* VP = (GEngine && GEngine->GameViewport) ? GEngine->GameViewport->Viewport : nullptr;
	if (!VP || VP->GetSizeXY().X <= 0 || VP->GetSizeXY().Y <= 0)
	{
		TORO_REPORT_ERROR(TEXT("No game viewport client found."));
		co_return;
	}

	if (ResolutionScale != 1.0f && bIncludeUI)
	{
		TORO_REPORT_WARNING(TEXT("Abnormal resolution scales do not support UI capture."));
		co_return;
	}

	auto CapturedOrProcessed = UE5Coro::Race(AwaitScreenshotCapture(Image), AwaitScreenshotProcessed());
	if (ResolutionScale == 1.0f)
	{
		FScreenshotRequest::RequestScreenshot(bIncludeUI, true);
	}
	else
	{
		const FIntPoint Size = VP->GetSizeXY();
		if ((Size.X * ResolutionScale) < 1.0f || (Size.Y * ResolutionScale) < 1.0f)
		{
			TORO_REPORT_ERROR(TEXT("Resolution scale too low. <1 pixel on one or both axis."));
			co_return;
		}

		if (!GetHighResScreenshotConfig().SetResolution(Size.X, Size.Y, ResolutionScale) || !VP->TakeHighResScreenShot())
		{
			co_return;
		}
	}

	co_await CapturedOrProcessed;
}

bool UToroImageLibrary::IsValidImageData(const FToroImageData& InData)
{
	return InData.IsValid();
}

void UToroImageLibrary::ClearImageData(FToroImageData& InData)
{
	InData.Clear();
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
	OutData.Clear();
	if (!IsValid(Target))
	{
		return false;
	}

	const FTexturePlatformData* PlatformData = Target->GetPlatformData();
	if (!PlatformData || PlatformData->Mips.IsEmpty())
	{
		TORO_REPORT_ERROR(TEXT("Invalid PlatformData or no Mips."));
		return false;
	}

	if (PlatformData->PixelFormat != SupportedFormat)
	{
		TORO_REPORT_ERROR(TEXT("Unsupported pixel format."));
		return false;
	}

	const auto& Mip = PlatformData->Mips[0];
	const int64 PixelCount = static_cast<int64>(Mip.SizeX) * static_cast<int64>(Mip.SizeY);
	if (Mip.SizeX <= 0
		|| Mip.SizeY <= 0
		|| PixelCount > MAX_int32
		|| Mip.BulkData.GetBulkDataSize() < PixelCount * static_cast<int64>(sizeof(FColor)))
	{
		TORO_REPORT_ERROR(TEXT("Invalid pixel count."));
		return false;
	}

	const void* TexData = Mip.BulkData.LockReadOnly();
	ON_SCOPE_EXIT
		{
			Mip.BulkData.Unlock();
		};
	if (!TexData)
	{
		TORO_REPORT_ERROR(TEXT("Failed to lock BulkData."));
		OutData.Clear();
		return false;
	}

	OutData.Size = FIntPoint(Mip.SizeX, Mip.SizeY);
	OutData.Pixels.SetNumUninitialized(static_cast<int32>(PixelCount));
	FMemory::Memcpy(OutData.Pixels.GetData(), TexData, static_cast<SIZE_T>(PixelCount) * sizeof(FColor));
	return true;
}

bool UToroImageLibrary::GetDataFromRenderTarget(FToroImageData& OutData, UTextureRenderTarget2D* Target, bool bInvertAlpha)
{
	OutData.Clear();
	if (!IsValid(Target))
	{
		return false;
	}

	const int64 PixelCount = static_cast<int64>(Target->SizeX) * static_cast<int64>(Target->SizeY);
	if (Target->SizeX <= 0
		|| Target->SizeY <= 0
		|| PixelCount > TNumericLimits<int32>::Max())
	{
		TORO_REPORT_ERROR(TEXT("Invalid pixel count."));
		return false;
	}

	if (FTextureRenderTargetResource* Resource = Target->GameThread_GetRenderTargetResource())
	{
		if (!Resource->ReadPixels(OutData.Pixels))
		{
			TORO_REPORT_ERROR(TEXT("Failed to read pixels."));
			OutData.Clear();
			return false;
		}

		OutData.Size = FIntPoint(Target->SizeX, Target->SizeY);
		if (!OutData.IsValid())
		{
			TORO_REPORT_ERROR(TEXT("Pixel count and data size mismatch."));
			OutData.Clear();
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

	TORO_REPORT_ERROR(TEXT("Failed to obtain resource."));
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
		TORO_REPORT_ERROR(TEXT("Failed to create texture."));
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
		TORO_REPORT_ERROR(TEXT("Invalid PlatformData or no Mips."));
		Image->MarkAsGarbage();
		return nullptr;
	}

	FTexture2DMipMap& Mip = PlatformData->Mips[0];
	void* TextureData = Mip.BulkData.Lock(LOCK_READ_WRITE);
	if (!TextureData)
	{
		TORO_REPORT_ERROR(TEXT("Failed to lock BulkData."));
		Mip.BulkData.Unlock();
		Image->MarkAsGarbage();
		return nullptr;
	}

	FMemory::Memcpy(TextureData, InData.Pixels.GetData(), sizeof(FColor) * static_cast<SIZE_T>(InData.Pixels.Num()));
	Mip.BulkData.Unlock();

	Image->UpdateResource();

	return Image;
}

FVoidCoroutine UToroImageLibrary::SaveTextureToFile(FLatentActionInfo LatentInfo,
	bool& bSuccess, const UTexture2D* Target, const FString& FilePath, bool bAsync)
{
	FToroImageData Data;
	if (!GetDataFromTexture(Data, Target))
	{
		bSuccess = false;
		co_return;
	}

	co_await SaveImageDataToFile(FLatentActionInfo(), bSuccess, Data, FilePath, bAsync);
}

FVoidCoroutine UToroImageLibrary::SaveRenderTargetToFile(FLatentActionInfo LatentInfo,
	bool& bSuccess, UTextureRenderTarget2D* Target, const FString& FilePath, bool bInvertAlpha, bool bAsync)
{
	FToroImageData Data;
	if (!GetDataFromRenderTarget(Data, Target, bInvertAlpha))
	{
		bSuccess = false;
		co_return;
	}

	co_await SaveImageDataToFile(FLatentActionInfo(), bSuccess, Data, FilePath, bAsync);
}

FVoidCoroutine UToroImageLibrary::SaveImageDataToFile(FLatentActionInfo LatentInfo,
	bool& bSuccess, const FToroImageData& InData, const FString& FilePath, bool bAsync)
{
	if (!FPaths::ValidatePath(FilePath))
	{
		TORO_REPORT_ERROR(TEXT("Invalid export path."));
		bSuccess = false;
		co_return;
	}

	TArray64<uint8> PNGImage;
	if (!InData.CompressPNG(PNGImage))
	{
		TORO_REPORT_ERROR(TEXT("Failed to compress PNG."));
		bSuccess = false;
		co_return;
	}

	if (bAsync)
	{
		const FString OutputFilePath = FilePath;
		co_await UE5Coro::Async::MoveToTask();
		const bool bResult = FFileHelper::SaveArrayToFile(PNGImage, *OutputFilePath);
		co_await UE5Coro::Async::MoveToGameThread();
		bSuccess = bResult;
		co_return;
	}

	bSuccess = FFileHelper::SaveArrayToFile(PNGImage, *FilePath);
}
