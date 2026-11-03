// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "SaveSystem/SaveGameArchive.h"
#include "UObject/SoftObjectPath.h"
#include "UObject/SoftObjectPtr.h"

FArchive& FSaveGameProxyArchive::operator<<(FSoftObjectPath& Value)
{
	FString Path;
	if (IsSaving())
	{
		Path = Value.ToString();
	}

	*this << Path;

	if (IsLoading() && !IsError())
	{
		Value.SetPath(Path);
	}

	return *this;
}

FArchive& FSaveGameProxyArchive::operator<<(FSoftObjectPtr& Value)
{
	FSoftObjectPath Path;
	if (IsSaving())
	{
		Path = Value.ToSoftObjectPath();
	}

	*this << Path;

	if (IsLoading() && !IsError())
	{
		Value = FSoftObjectPtr(Path);
	}

	return *this;
}

FArchive& FSaveGameProxyArchive::operator<<(FWeakObjectPtr&)
{
	SetError();
	ensureMsgf(false, TEXT("Serializing object pointers is unsupported. Use Obj->SerializeScriptProperties(Ar)"));
	return *this;
}

FArchive& FSaveGameProxyArchive::operator<<(FObjectPtr&)
{
	SetError();
	ensureMsgf(false, TEXT("Serializing object pointers is unsupported. Use Obj->SerializeScriptProperties(Ar)"));
	return *this;
}

FArchive& FSaveGameProxyArchive::operator<<(UObject*&)
{
	SetError();
	ensureMsgf(false, TEXT("Serializing object pointers is unsupported. Use Obj->SerializeScriptProperties(Ar)"));
	return *this;
}
