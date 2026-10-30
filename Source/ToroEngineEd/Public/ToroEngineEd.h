// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "Containers/UnrealString.h"
#include "Modules/ModuleInterface.h"

/**
 * Editor extensions for the ToroEngine runtime framework.
 */
class FToroEngineEdModule final : public IModuleInterface
{
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

public:

	TOROENGINEED_API static FString GetPluginDir();
};
