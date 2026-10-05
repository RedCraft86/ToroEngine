// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#include "Libraries/ToroUtilityLibrary.h"
#include "Compression/OodleDataCompressionUtil.h"
#include "UObject/UObjectGlobals.h"
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

bool UToroUtilityLibrary::IsAnyPackageLoading()
{
	return IsLoading();
}

int32 UToroUtilityLibrary::GetNumAsyncPackages()
{
	return ::GetNumAsyncPackages();
}

bool UToroUtilityLibrary::OodleCompress(const TArray<uint8>& InData, TArray<uint8>& OutData,
	const EOodleCompressor Compressor, const EOodleCompressionLevel Level)
{
	TArray<uint8> Compressed;
	const bool bSuccess = FOodleCompressedArray::CompressTArray(Compressed, InData,
		OodleCompressorToNative(Compressor), OodleCompressionLevelToNative(Level));

	if (bSuccess)
	{
		OutData = MoveTemp(Compressed);
	}
	else
	{
		OutData.Empty();
	}

	return bSuccess;
}

bool UToroUtilityLibrary::OodleDecompress(const TArray<uint8>& InData, TArray<uint8>& OutData)
{
	TArray<uint8> Decompressed;
	const bool bSuccess = !InData.IsEmpty() 
		&& FOodleCompressedArray::DecompressToTArray(Decompressed, InData);

	if (bSuccess)
	{
		OutData = MoveTemp(Decompressed);
	}
	else
	{
		OutData.Empty();
	}

	return bSuccess;
}
