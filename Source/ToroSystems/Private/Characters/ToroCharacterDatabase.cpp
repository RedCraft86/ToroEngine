// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "Characters/ToroCharacterDatabase.h"

namespace ToroCharacterTags
{
	UE_DEFINE_GAMEPLAY_TAG(Base, "Character");
	UE_DEFINE_GAMEPLAY_TAG(Player, "Character.Player");
}

UToroCharacterDatabase::UToroCharacterDatabase()
	: Super(
		ToroCharacterTags::Base.GetTag(),
		FToroCharacterEntry::StaticStruct()
	)
{
}
