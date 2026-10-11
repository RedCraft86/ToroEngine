// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#pragma once

#include "MiscEditor/ToroSlateStyle.h"

class FToroEngineStyle final : public FToroSlateStyle
{
public:

	static inline const FName StyleName = TEXT("ToroEngineStyle");

	FToroEngineStyle()
		: FToroSlateStyle(StyleName)
	{}

private:

	virtual void AddResources() override
	{
		SetContentRoot(FToroCoreEdModule::GetPluginDir() / TEXT("Resources"));

		AddSVG(TEXT("Toolbar.ActorSplit"), TEXT("Toolbar/ActorSplit"), Icon20x20);
		AddSVG(TEXT("Toolbar.ActorInstance"), TEXT("Toolbar/ActorInstance"), Icon20x20);
		AddSVG(TEXT("Toolbar.ActorLayout"), TEXT("Toolbar/ActorLayout"), Icon20x20);
		AddSVG(TEXT("Toolbar.ActorMerge"), TEXT("Toolbar/ActorMerge"), Icon20x20);
	}
};
