// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#pragma once

#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(LogToroEngineEd, All, All);

/**
 * Editor support for ToroEngine features and standalone editor tools,
 * including authoring workflows, actor operations, and engine extensions.
 * Built upon ToroCoreEd and ToroEngine.
 */
class FToroEngineEdModule final : public IModuleInterface
{
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
};
