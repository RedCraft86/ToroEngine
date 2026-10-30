// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "ToroEngineEd.h"
#include "SlateStyles/ToroSlateStyle.h"

class FToroSystemsStyle final : public FToroSlateStyle
{
public:

	static inline const FName StyleName = TEXT("ToroSystemsStyle");

	FToroSystemsStyle()
		: FToroSlateStyle(StyleName)
	{}

private:

	virtual void AddResources() override
	{
		SetContentRoot(FToroEngineEdModule::GetPluginDir() / TEXT("Resources"));

		AddPNG(TEXT("ClassThumbnail.LightProbe"), TEXT("Class/LightProbe"), Icon64x64);
		AddPNG(TEXT("ClassThumbnail.LightProbeComponent"), TEXT("Class/LightProbe"), Icon64x64);
		AddPNG(TEXT("ClassIcon.LightProbe"), TEXT("Class/LightProbe"), Icon16x16);
		AddPNG(TEXT("ClassIcon.LightProbeComponent"), TEXT("Class/LightProbe"), Icon16x16);
	}
};
