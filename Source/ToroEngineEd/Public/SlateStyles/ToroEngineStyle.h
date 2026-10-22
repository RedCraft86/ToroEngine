// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "ToroEngineEd.h"
#include "ToroSlateStyle.h"

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
		SetContentRoot(FToroEngineEdModule::GetPluginDir() / TEXT("Resources"));
	}
};
