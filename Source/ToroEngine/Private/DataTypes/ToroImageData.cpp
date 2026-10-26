// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "DataTypes/ToroImageData.h"
#include "ImageUtils.h"

bool FToroImageData::CompressPNG(TArray64<uint8>& OutData) const
{
	OutData.Empty();
	if (IsValid())
	{
		FImageUtils::PNGCompressImageArray(Size.X, Size.Y, Pixels, OutData);
		return !OutData.IsEmpty();
	}

	return false;
}

void FToroImageData::Clear()
{
	Size = FIntPoint::ZeroValue;
	Pixels.Empty();
}
