// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "Serialization/NameAsStringProxyArchive.h"

struct TOROSYSTEMS_API FSaveGameProxyArchive final : FNameAsStringProxyArchive
{
	FSaveGameProxyArchive(FArchive& InInnerArchive)
		: FNameAsStringProxyArchive(InInnerArchive)
	{
		ArIsSaveGame = true; // Only serialize properties marked with the "SaveGame" UPROPERTY specifier.
		ArNoDelta = true; // Save all properties, even if they are unchanged from CDO.
	}

	virtual FArchive& operator<<(FSoftObjectPath& Value) override;
	virtual FArchive& operator<<(FSoftObjectPtr& Value) override;
	virtual FArchive& operator<<(FWeakObjectPtr& Obj) override;
	virtual FArchive& operator<<(FObjectPtr& Obj) override;
	virtual FArchive& operator<<(UObject*& Obj) override;
};
