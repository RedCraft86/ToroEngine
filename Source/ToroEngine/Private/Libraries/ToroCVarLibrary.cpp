// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "Libraries/ToroCVarLibrary.h"

namespace
{
	/** Internal templated function to set a console variable with library min/max priorities. */
	template<typename T>
	bool SetConsoleVar(const FString& InName, const T& InValue)
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

bool UToroCVarLibrary::SetCVarBool(const FString& InName, bool bInValue)
{
	return SetConsoleVar(InName, bInValue);
}

bool UToroCVarLibrary::SetCVarInt(const FString& InName, int32 InValue)
{
	return SetConsoleVar(InName, InValue);
}

bool UToroCVarLibrary::SetCVarFloat(const FString& InName, float InValue)
{
	return SetConsoleVar(InName, InValue);
}

bool UToroCVarLibrary::SetCVarString(const FString& InName, const FString& InValue)
{
	return SetConsoleVar(InName, *InValue);
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
