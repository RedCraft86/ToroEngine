// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "Utilities/ToroValidation.h"
#include "Logging/TokenizedMessage.h"
#include "Internationalization/Text.h"
#include "Misc/DataValidation.h"
#include "GameFramework/Actor.h"
#include "Logging/MessageLog.h"
#include "Misc/UObjectToken.h"
#include "UObject/Object.h"
#include "Engine/World.h"

#define LOCTEXT_NAMESPACE "ToroEngine"
namespace ToroEngine::Validation
{
	namespace
	{
		/**
		 * Runs manual object validation, forwards its issues, and opens a new Message Log page.
		 * Only an invalid result or reported errors produce an error summary; warnings keep their original severity.
		 * @param Target Valid object to validate through IsDataValid().
		 * @param LogName Destination log; NAME_None selects the default data-validation log.
		 * @param bUsePathName Whether the page title uses the object path instead of its short name.
		 * @param TypeName Localized object-type label used in page titles and summaries.
		 */
		void ToroValidateInternal(const UObject& Target, FName LogName, bool bUsePathName, const FText& TypeName)
		{
			const FName& MsgLogName = LogName.IsNone() ? UE::DataValidation::MessageLogName : LogName;

			FMessageLog Log(MsgLogName);
			Log.NewPage(FText::Format(LOCTEXT("ValidationPage", "Validate {0}: {1}"),
				TypeName, FText::FromString(bUsePathName ? Target.GetPathName() : Target.GetName())
			));

			FDataValidationContext Context(false, EDataValidationUsecase::Manual, {}, MsgLogName);
			const EDataValidationResult ValidateResult = Target.IsDataValid(Context);
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

			TSharedPtr<FTokenizedMessage> ResultMessage;
			if (ValidateResult == EDataValidationResult::Invalid || Context.GetNumErrors() > 0)
			{
				ResultMessage = Log.Error(FText::Format(
					LOCTEXT("ValidationFailed", "{0} validation failed with {1} errors and {2} warnings."),
					TypeName, FText::AsNumber(Context.GetNumErrors()), FText::AsNumber(Context.GetNumWarnings())
				)).ToSharedPtr();
			}
			else if (ValidateResult == EDataValidationResult::Valid)
			{
				ResultMessage = Log.Info(FText::Format(
					LOCTEXT("ValidationPassed", "{0} validation passed."), TypeName
				)).ToSharedPtr();
			}
			else
			{
				ResultMessage = Log.Info(FText::Format(
					LOCTEXT("ValidationNotValidated", "{0} was not validated."), TypeName
				)).ToSharedPtr();
			}

			if (ResultMessage.IsValid())
			{
				ResultMessage->AddToken(FUObjectToken::Create(&Target));
			}

			Log.Open(EMessageSeverity::Info, true);
		}
	}

	void ValidateObject(const UObject* Target, FName LogName)
	{
		if (IsValid(Target))
		{
			ToroValidateInternal(*Target, LogName, true, LOCTEXT("ValidationObjectType", "Object"));
		}
	}

	void ValidateAsset(const UObject* Target)
	{
		if (IsValid(Target))
		{
			ToroValidateInternal(*Target, TEXT("AssetCheck"), true, LOCTEXT("ValidationAssetType", "Asset"));
		}
	}

	void ValidateActor(const AActor* Target)
	{
		if (IsValid(Target))
		{
			ToroValidateInternal(*Target, TEXT("MapCheck"), true, LOCTEXT("ValidationActorType", "Actor"));
		}
	}

	void ValidateWorld(const UWorld* Target)
	{
		if (IsValid(Target))
		{
			ToroValidateInternal(*Target, TEXT("MapCheck"), false, LOCTEXT("ValidationWorldType", "World"));
		}
	}
}
#undef LOCTEXT_NAMESPACE