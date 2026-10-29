// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "Libraries/ToroStringLibrary.h"
#include "Internationalization/StringTableCore.h"
#include "Misc/Paths.h"

bool UToroStringLibrary::IsNameNone(FName InName)
{
	return InName.IsNone();
}

FString UToroStringLibrary::NameToDisplayString(FName InName)
{
	return FName::NameToDisplayString(InName.ToString(), false);
}

bool UToroStringLibrary::IsStringEmptyOrWhitespace(const FString& InString)
{
	return InString.TrimStartAndEnd().IsEmpty();
}

void UToroStringLibrary::StringAppendTo(FString& InString, const FString& Appendage)
{
	InString.Append(Appendage);
}

void UToroStringLibrary::StringAppendToMultiple(FString& InString, const TArray<FString>& Appendages)
{
	for (const FString& Appendage : Appendages)
	{
		InString.Append(Appendage);
	}
}

FString UToroStringLibrary::CombinePaths(const FString& A, const FString& B)
{
	return FPaths::Combine(A, B);
}

bool UToroStringLibrary::IsTextEmptyOrWhitespace(FText InText)
{
	return InText.IsEmptyOrWhitespace();
}

bool UToroStringLibrary::IsTextStringTablePlaceholder(FText InText)
{
	return InText.ToString() == FStringTableEntry::GetPlaceholderSourceString();
}
