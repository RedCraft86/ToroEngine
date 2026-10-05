// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#include "DataTypes/ToroImageData.h"
#include "ImageUtils.h"

bool FToroImageData::CompressPNG(TArray64<uint8>& OutData) const
{
	if (IsValid())
	{
		FImageUtils::PNGCompressImageArray(Size.X, Size.Y, Pixels, OutData);
		return !OutData.IsEmpty();
	}

	OutData.Empty();
	return false;
}

void FToroImageData::Empty()
{
	Size = FIntPoint::ZeroValue;
	Pixels.Empty();
}
