// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "Modules/ModuleInterface.h"

/**
 * Editor extensions for ToroSystems, built on ToroEngineEd.
 */
class FToroSystemsEdModule final : public IModuleInterface
{
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
