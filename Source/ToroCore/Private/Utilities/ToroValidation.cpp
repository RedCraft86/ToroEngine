// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#include "Utilities/ToroValidation.h"

#if WITH_EDITOR
#include "MessageLogModule.h"
#include "IMessageLogListing.h"
#include "Modules/ModuleManager.h"
#include "GameFramework/Actor.h"
#include "Misc/DataValidation.h"
#include "Logging/MessageLog.h"

#define LOCTEXT_NAMESPACE "ToroEngine"

namespace
{
	void ValidateInternal(const UObject& Target, const FName& LogName, const FText& ObjectType, bool bClearPages = false)
	{
		const FName& MsgLogName = LogName.IsNone() ? UE::DataValidation::MessageLogName : LogName;
		if (bClearPages)
		{
			FMessageLogModule& Module = FModuleManager::LoadModuleChecked<FMessageLogModule>(TEXT("MessageLog"));
			Module.GetLogListing(MsgLogName)->ClearMessages();
		}

		FMessageLog Log(MsgLogName);
		Log.NewPage(FText::Format(
			LOCTEXT("ValidationPage", "Validate {0}: {1}"),
			ObjectType,
			FText::FromString(Target.GetPathName())
		));

		FDataValidationContext Context(false, EDataValidationUsecase::Manual, {}, MsgLogName);
		const EDataValidationResult Result = Target.IsDataValid(Context);

		for (const FDataValidationContext::FIssue& Issue : Context.GetIssues())
		{
			if (Issue.TokenizedMessage.IsValid())
			{
				Log.AddMessage(Issue.TokenizedMessage.ToSharedRef());
			}
			else
			{
				Log.Message(Issue.Severity, Issue.Message);
			}
		}

		if (Result == EDataValidationResult::Invalid || Context.GetNumErrors() > 0 || Context.GetNumWarnings() > 0)
		{
			Log.Error(FText::Format(LOCTEXT("ValidationFailed", "{0} Validation failed with {1} Errors and {2} Warnings."),
				ObjectType, FText::AsNumber(Context.GetNumErrors()), FText::AsNumber(Context.GetNumWarnings())
			));
		}
		else if (Result == EDataValidationResult::Valid)
		{
			Log.Info(FText::Format(LOCTEXT("ValidationPassed", "{0} Validation passed."), ObjectType));
		}
		else
		{
			Log.Info(FText::Format(LOCTEXT("ValidationNotValidated", "{0} was not validated."), ObjectType));
		}

		Log.Open(EMessageSeverity::Info, true);
	}
}

void ToroEngine::Validation::ValidateWorld(const UWorld* Target)
{
	if (IsValid(Target))
	{
		ValidateInternal(*Target, TEXT("MapCheck"), LOCTEXT("ValidationWorldLabel", "World"), true);
	}
}

void ToroEngine::Validation::ValidateActor(const AActor* Target)
{
	if (IsValid(Target))
	{
		ValidateInternal(*Target, TEXT("MapCheck"), LOCTEXT("ValidationActorLabel", "Actor"));
	}
}

void ToroEngine::Validation::ValidateAsset(const UObject* Target)
{
	if (IsValid(Target))
	{
		ValidateInternal(*Target, UE::DataValidation::MessageLogName, LOCTEXT("ValidationAssetLabel", "Asset"));
	}
}

void ToroEngine::Validation::ValidateObject(const UObject* Target, FName LogName)
{
	if (IsValid(Target))
	{
		ValidateInternal(*Target, LogName, LOCTEXT("ValidationObjectLabel", "Object"));
	}
}

#undef LOCTEXT_NAMESPACE
#endif
