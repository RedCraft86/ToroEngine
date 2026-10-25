// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "Logging/LogMacros.h"
#include "Modules/ModuleInterface.h"

TOROENGINE_API DECLARE_LOG_CATEGORY_EXTERN(LogToroEngine, All, All)

/**
 * Base runtime module for the ToroEngine framework.
 */
class FToroEngineModule final : public IModuleInterface
{
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
