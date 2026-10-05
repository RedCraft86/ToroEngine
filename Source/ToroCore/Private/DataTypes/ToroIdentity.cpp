// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#include "DataTypes/ToroIdentity.h"

const FToroIdentity FToroIdentity::Empty = FToroIdentity();

FString FToroIdentity::ToString() const
{
	return FString::Printf(TEXT("%s[%s]"), *Group.ToString(), *Guid.ToString());
}

void FToroIdentity::RegenerateGuid(bool bOnlyIfInvalid)
{
	if (!bOnlyIfInvalid || !Guid.IsValid())
	{
		Guid = FGuid::NewGuid();
	}
}

void FToroIdentity::Invalidate()
{
	Group = FGameplayTag::EmptyTag;
	Guid.Invalidate();
}
