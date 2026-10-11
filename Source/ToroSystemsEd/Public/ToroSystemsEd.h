// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#pragma once

#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(LogToroSystemsEd, All, All);

/**
 * Editor support for ToroSystems, including system-specific authoring,
 * configuration, visualization, and validation workflows.
 * Built upon shared editor infrastructure and the systems it supports.
 */
class FToroSystemsEdModule final : public IModuleInterface
{
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
};
