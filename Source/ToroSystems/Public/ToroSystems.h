// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "Modules/ModuleInterface.h"

/**
 * Runtime systems built on the ToroEngine framework.
 */
class FToroSystemsModule final : public IModuleInterface
{
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
