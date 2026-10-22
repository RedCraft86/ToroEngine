// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "Logging/LogMacros.h"
#include "Modules/ModuleInterface.h"

TOROSYSTEMSED_API DECLARE_LOG_CATEGORY_EXTERN(LogToroSystemsEd, All, All)

/**
 * Editor extensions for ToroSystems, built on ToroEngineEd.
 */
class FToroSystemsEdModule final : public IModuleInterface
{
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
