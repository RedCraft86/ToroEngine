// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#pragma once

#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(LogToroSystems, All, All);

/**
 * Complete systems that coordinate related functionality into cohesive gameplay
 * or application workflows, built upon UnrealEngine, ToroCore, and ToroEngine.
 */
class FToroSystemsModule final : public IModuleInterface
{
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
};
