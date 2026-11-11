// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "DataTypes/ToroLevelInfo.h"
#include "Utilities/ToroUtilities.h"
#include "ToroSystemSettings.h"
#include "UObject/Package.h"
#include "Engine/World.h"

namespace LevelTypeTags
{
	UE_DEFINE_GAMEPLAY_TAG(Base, "LevelType");
	UE_DEFINE_GAMEPLAY_TAG(MainMenu, "LevelType.MainMenu");
	UE_DEFINE_GAMEPLAY_TAG(Gameplay, "LevelType.Gameplay");
}

const FToroLevelInfo* FToroLevelInfo::Get(const UObject* Context)
{
	const UWorld* World = ToroEngine::GetWorld(Context);
	if (!World)
	{
		return nullptr;
	}

	const FString CurrentPackage = UWorld::RemovePIEPrefix(World->GetOutermost()->GetName());
	for (const TPair<TSoftObjectPtr<UWorld>, FToroLevelInfo>& Entry : UToroSystemSettings::Get()->Levels)
	{
		if (!Entry.Key.IsNull()
			&& Entry.Value.IsValid()
			&& Entry.Key.ToSoftObjectPath().GetLongPackageName() == CurrentPackage)
		{
			return &Entry.Value;
		}
	}

	return nullptr;
}

