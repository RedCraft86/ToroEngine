// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#pragma once

#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(LogToroCore, All, All);

/**
 * Reusable types, base classes, abstractions, and utility functions.
 * Provides building blocks without defining standalone gameplay or engine features.
 */
class FToroCoreModule final : public IModuleInterface
{
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

	FDelegateHandle PreLoadMapHandle;
	static void PreLoadMap(const FString& Map);

	FDelegateHandle PostLoadMapHandle;
	static void PostLoadMap(UWorld* World);
};
