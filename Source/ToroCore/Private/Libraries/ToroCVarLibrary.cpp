// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#include "Libraries/ToroCVarLibrary.h"

namespace
{
	/**
	 * Submits a typed write using the library's priority bounds; reports lookup success.
	 */
	template<typename T>
	bool SetCVar(const FString& InName, const T& InValue)
	{
		if (IConsoleVariable* CVar = UToroCVarLibrary::FindCVar(InName))
		{
			CVar->SetWithCurrentPriority(
				InValue, NAME_None,
				UToroCVarLibrary::MaxPriority,
				UToroCVarLibrary::MinPriority
			);

			return true;
		}

		return false;
	}
}

bool UToroCVarLibrary::SetCVarBool(const FString& InName, bool InValue)
{
	return SetCVar(InName, InValue);
}

bool UToroCVarLibrary::SetCVarInt(const FString& InName, int32 InValue)
{
	return SetCVar(InName, InValue);
}

bool UToroCVarLibrary::SetCVarFloat(const FString& InName, float InValue)
{
	return SetCVar(InName, InValue);
}

bool UToroCVarLibrary::SetCVarString(const FString& InName, const FString& InValue)
{
	return SetCVar(InName, *InValue);
}

bool UToroCVarLibrary::GetCVarBool(const FString& InName, bool bInDefault)
{
	const IConsoleVariable* CVar = FindCVar(InName);
	return CVar ? CVar->GetBool() : bInDefault;
}

int32 UToroCVarLibrary::GetCVarInt(const FString& InName, int32 InDefault)
{
	const IConsoleVariable* CVar = FindCVar(InName);
	return CVar ? CVar->GetInt() : InDefault;
}

float UToroCVarLibrary::GetCVarFloat(const FString& InName, float InDefault)
{
	const IConsoleVariable* CVar = FindCVar(InName);
	return CVar ? CVar->GetFloat() : InDefault;
}

FString UToroCVarLibrary::GetCVarString(const FString& InName, const FString& InDefault)
{
	const IConsoleVariable* CVar = FindCVar(InName);
	return CVar ? CVar->GetString() : InDefault;
}

IConsoleVariable* UToroCVarLibrary::FindCVar(const FString& InName)
{
	return IConsoleManager::Get().FindConsoleVariable(*InName);
}
