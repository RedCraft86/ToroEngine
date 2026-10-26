// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "Libraries/ToroUtilityLibrary.h"
#include "Compression/OodleDataCompressionUtil.h"
#include "UObject/UObjectGlobals.h"
#include "ShaderPipelineCache.h"
#if WITH_EDITOR
#include "Misc/App.h"
#endif

bool UToroUtilityLibrary::IsInGame()
{
#if WITH_EDITOR
	return FApp::IsGame();
#else
	return true;
#endif
}

bool UToroUtilityLibrary::IsAsyncLoading()
{
	return ::IsAsyncLoading();
}

int32 UToroUtilityLibrary::GetNumAsyncPackages()
{
	return ::GetNumAsyncPackages();
}

void UToroUtilityLibrary::PausePrecompilingPSOs()
{
	FShaderPipelineCache::PauseBatching();
}

void UToroUtilityLibrary::ResumePrecompilingPSOs()
{
	FShaderPipelineCache::ResumeBatching();
}

bool UToroUtilityLibrary::OodleCompress(const TArray<uint8>& InData, TArray<uint8>& OutData,
	TEnumAsByte<EOodleCompressor::Type> Compressor, TEnumAsByte<EOodleCompressionLevel::Type> Level)
{
	TArray<uint8> Compressed;
	if (FOodleCompressedArray::CompressTArray(Compressed, InData,
		EOodleCompressor::ToNative(Compressor), EOodleCompressionLevel::ToNative(Level)))
	{
		OutData = MoveTemp(Compressed);
		return true;
	}

	OutData.Empty();
	return false;
}

bool UToroUtilityLibrary::OodleDecompress(const TArray<uint8>& InData, TArray<uint8>& OutData)
{
	TArray<uint8> Decompressed;
	if (FOodleCompressedArray::DecompressToTArray(Decompressed, InData))
	{
		OutData = MoveTemp(Decompressed);
		return true;
	}

	OutData.Empty();
	return false;
}
