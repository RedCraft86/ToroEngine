// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "Utilities/ToroLogging.h"
#include "Misc/AssertionMacros.h"
#include "UObject/Stack.h"
#include "ToroEngine.h"
#if WITH_EDITOR
#include "CoreGlobals.h"
#include "Editor.h"
#endif

namespace ToroEngine::Logging::Private
{
	void LogInfo(const FString& Message)
	{
		UE_LOG(LogToroEngine, Display, TEXT("%s"), *Message);
	}

	void LogWarning(const FString& Message)
	{
		UE_LOG(LogToroEngine, Warning, TEXT("%s"), *Message);
	}

	void LogError(const FString& Message)
	{
		UE_LOG(LogToroEngine, Error, TEXT("%s"), *Message);
	}

	void LogFatal(const FString& Message)
	{
#if WITH_EDITOR
		if (GIsEditor)
		{
			LogError(Message);
			if (GEditor)
			{
				GEditor->RequestEndPlayMap();
			}
			return;
		}
#endif
		UE_LOG(LogToroEngine, Fatal, TEXT("%s"), *Message);
	}

	void ReportWarning(const FString& Message)
	{
		if (FFrame::GetThreadLocalTopStackFrame())
		{
			FFrame::KismetExecutionMessage(*Message, ELogVerbosity::Warning);
		}
		else
		{
			LogWarning(Message);
		}
	}

	void ReportError(const FString& Message)
	{
		if (FFrame::GetThreadLocalTopStackFrame())
		{
			FFrame::KismetExecutionMessage(*Message, ELogVerbosity::Error);
		}
		else
		{
			ensureAlwaysMsgf(false, TEXT("%s"), *Message);
		}
	}

	void ReportFatal(const FString& Message)
	{
#if WITH_EDITOR
		if (GIsEditor)
		{
			ReportError(Message);
			if (GEditor)
			{
				GEditor->RequestEndPlayMap();
			}
			return;
		}
#endif
		FFrame::KismetExecutionMessage(*Message, ELogVerbosity::Fatal);
	}
}
