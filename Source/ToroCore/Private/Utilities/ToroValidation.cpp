// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#include "Utilities/ToroValidation.h"

#if WITH_EDITOR
#include "GameFramework/Actor.h"
#include "Misc/DataValidation.h"
#include "Logging/MessageLog.h"

#define LOCTEXT_NAMESPACE "ToroEngine"

namespace
{
	void ValidateInternal(const UObject* Target, const FName& LogName, const FText& ObjectType)
	{
		const FName& MsgLogName = LogName.IsNone() ? UE::DataValidation::MessageLogName : LogName;

		FMessageLog Log(MsgLogName);
		Log.NewPage(FText::Format(
			LOCTEXT("ValidationPage", "Validate {0}: {1}"),
			ObjectType, FText::FromString(Target->GetPathName())
		));

		FDataValidationContext Context(false, EDataValidationUsecase::Manual, {}, MsgLogName);
		const EDataValidationResult Result = Target->IsDataValid(Context);

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

		if (Result == EDataValidationResult::Invalid || Context.GetNumErrors() > 0)
		{
			Log.Error(FText::Format(LOCTEXT("ValidationFailed", "{0} Validation failed."), ObjectType));
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

void ToroEngine::Validation::ValidateActor(const AActor* Target)
{
	if (IsValid(Target))
	{
		ValidateInternal(Target, "MapCheck", LOCTEXT("ValidationActorLabel", "Actor"));
	}
}

void ToroEngine::Validation::ValidateAsset(const UObject* Target)
{
	if (IsValid(Target))
	{
		ValidateInternal(Target, UE::DataValidation::MessageLogName, LOCTEXT("ValidationAssetLabel", "Asset"));
	}
}

void ToroEngine::Validation::ValidateObject(const UObject* Target, const FName LogName)
{
	if (IsValid(Target))
	{
		ValidateInternal(Target, LogName, LOCTEXT("ValidationObjectLabel", "Object"));
	}
}

#undef LOCTEXT_NAMESPACE
#endif
