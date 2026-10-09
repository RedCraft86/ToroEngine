// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#include "MiscEditor/ToroSlateStyle.h"
#include "Styling/SlateStyleMacros.h"

void FToroSlateStyle::AddPNG(const FName& Name, const FString& Path, const FVector2D& Size)
{
	Set(Name, new IMAGE_BRUSH(Path, Size));
}

void FToroSlateStyle::AddSVG(const FName& Name, const FString& Path, const FVector2D& Size)
{
	Set(Name, new IMAGE_BRUSH_SVG(Path, Size));
}
