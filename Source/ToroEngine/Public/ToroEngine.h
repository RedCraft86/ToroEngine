// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#pragma once

#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(LogToroEngine, All, All);

/**
 * Feature implementations and extended engine classes built upon UnrealEngine
 * and ToroCore, with their own behavior, configuration, and lifecycle.
 */
class FToroEngineModule final : public IModuleInterface
{
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
};
