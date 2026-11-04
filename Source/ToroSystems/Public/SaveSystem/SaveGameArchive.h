// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "Serialization/NameAsStringProxyArchive.h"

/**
 * Proxies an archive for SaveGame property serialization without delta serialization.
 * Names and soft object references are stored as strings; soft references are not resolved or loaded.
 * Weak and hard object pointers are unsupported and set the archive error flag when encountered.
 * Serialize an object's properties through SerializeScriptProperties instead of serializing its pointer.
 */
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
