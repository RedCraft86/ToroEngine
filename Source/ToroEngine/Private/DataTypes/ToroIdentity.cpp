// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "DataTypes/ToroIdentity.h"

const FToroIdentity FToroIdentity::Empty = FToroIdentity();

FString FToroIdentity::ToString() const
{
	return FString::Printf(TEXT("%s[%s]"), *Group.ToString(), *Guid.ToString());
}

void FToroIdentity::Invalidate()
{
	Group = FGameplayTag::EmptyTag;
	Guid.Invalidate();
}

void FToroIdentity::RegenerateGuid(bool bOnlyIfInvalid)
{
	if (!bOnlyIfInvalid || !Guid.IsValid())
	{
		Guid = FGuid::NewGuid();
	}
}

#if WITH_EDITOR
bool FToroIdentity::CanEditChange(const FEditPropertyChain& PropertyChain) const
{
	if (!Group.IsValid())
	{
		for (const FProperty* Property : PropertyChain)
		{
			if (Property && Property->GetFName() == GET_MEMBER_NAME_CHECKED(FToroIdentity, Guid))
			{
				return false;
			}
		}
	}

	return true;
}
#endif

bool UToroIdentityLibrary::IsIdentityValid(const FToroIdentity& Identity)
{
	return Identity.IsValid();
}

bool UToroIdentityLibrary::EqualIdentity(const FToroIdentity& A, const FToroIdentity& B)
{
	return A == B;
}

bool UToroIdentityLibrary::NotEqualIdentity(const FToroIdentity& A, const FToroIdentity& B)
{
	return A != B;
}

FString UToroIdentityLibrary::Conv_ToroIdentityToString(const FToroIdentity& Identity)
{
	return Identity.ToString();
}

void UToroIdentityLibrary::InvalidateIdentity(FToroIdentity& Identity)
{
	Identity.Invalidate();
}

void UToroIdentityLibrary::RegenerateIdentityGuid(FToroIdentity& Identity, bool bOnlyIfInvalid)
{
	Identity.RegenerateGuid(bOnlyIfInvalid);
}
