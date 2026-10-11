// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#pragma once

#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(LogToroCoreEd, All, All);

/**
 * Editor support for ToroCore types and reusable editor infrastructure,
 * including detail customizations, asset factories, component visualizers,
 * toolbar registration, and Slate style helpers.
 */
class FToroCoreEdModule final : public IModuleInterface
{
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
};
