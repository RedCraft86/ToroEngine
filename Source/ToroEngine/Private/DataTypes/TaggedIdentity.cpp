// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "DataTypes/TaggedIdentity.h"
#if WITH_EDITOR
#include "UObject/UnrealType.h"
#endif

const FTaggedIdentity FTaggedIdentity::Empty = FTaggedIdentity();

FString FTaggedIdentity::ToString() const
{
	return FString::Printf(TEXT("%s[%s]"), *Group.ToString(), *Guid.ToString());
}

void FTaggedIdentity::Invalidate()
{
	Group = FGameplayTag::EmptyTag;
	Guid.Invalidate();
}

void FTaggedIdentity::RegenerateGuid(bool bOnlyIfInvalid)
{
	if (!bOnlyIfInvalid || !Guid.IsValid())
	{
		Guid = FGuid::NewGuid();
	}
}

#if WITH_EDITOR
bool FTaggedIdentity::CanEditChange(const FEditPropertyChain& PropertyChain) const
{
	if (!Group.IsValid())
	{
		for (const FProperty* Property : PropertyChain)
		{
			if (Property && Property->GetFName() == GET_MEMBER_NAME_CHECKED(FTaggedIdentity, Guid))
			{
				return false;
			}
		}
	}

	return true;
}
#endif

bool UTaggedIdentityLibrary::IsIdentityValid(const FTaggedIdentity& Identity)
{
	return Identity.IsValid();
}

bool UTaggedIdentityLibrary::EqualIdentity(const FTaggedIdentity& A, const FTaggedIdentity& B)
{
	return A == B;
}

bool UTaggedIdentityLibrary::NotEqualIdentity(const FTaggedIdentity& A, const FTaggedIdentity& B)
{
	return A != B;
}

FString UTaggedIdentityLibrary::Conv_TaggedIdentityToString(const FTaggedIdentity& Identity)
{
	return Identity.ToString();
}

void UTaggedIdentityLibrary::InvalidateIdentity(FTaggedIdentity& Identity)
{
	Identity.Invalidate();
}

void UTaggedIdentityLibrary::RegenerateIdentityGuid(FTaggedIdentity& Identity, bool bOnlyIfInvalid)
{
	Identity.RegenerateGuid(bOnlyIfInvalid);
}
