// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "NativeGameplayTags.h"
#include "GameplayTagContainer.h"
#include "ToroLevelInfo.generated.h"

class UInputMappingContext;

namespace LevelTypeTags
{
	TOROSYSTEMS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Base);
	TOROSYSTEMS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MainMenu);
	TOROSYSTEMS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Gameplay);
}

/**
 * Per-map configuration stored in UToroSystemSettings, with static lookup for the current map.
 * Identifies a specific level type beneath the LevelType gameplay-tag root and optionally references
 * an input mapping context without loading it. A valid entry excludes the root tag itself.
 */
USTRUCT(BlueprintType)
struct TOROSYSTEMS_API FToroLevelInfo final
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = Preset, meta = (Categories = "LevelType"))
	FGameplayTag LevelType;

	UPROPERTY(EditAnywhere, Category = Preset)
	TSoftObjectPtr<UInputMappingContext> InputMappings;

	/** Whether LevelType is valid; mappings are optional. */
	[[nodiscard]] FORCEINLINE bool IsValid() const
	{
		return LevelType.IsValid()
			&& LevelType != LevelTypeTags::Base.GetTag()
			&& LevelType.MatchesTag(LevelTypeTags::Base);
	}

	/**
	 * Retrieves valid settings for the context world's current map without loading assets.
	 * @return Matching settings-owned entry that passes IsValid(), or nullptr if the world is unavailable,
	 *         the map is unconfigured, or its entry is invalid.
	 * @note Do not retain resulting pointer across edits to the settings map.
	 */
	[[nodiscard]] static const FToroLevelInfo* Get(const UObject* Context);
};
