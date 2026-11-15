// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "NativeGameplayTags.h"
#include "GameplayTagContainer.h"
#include "DataAssets/ToroDatabase.h"
#include "ToroCharacterDatabase.generated.h"

namespace ToroCharacterTags
{
	TOROSYSTEMS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Base);
	TOROSYSTEMS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Player);

	[[nodiscard]] inline bool IsValid(const FGameplayTag& InTag)
	{
		return InTag.IsValid()
			&& InTag != Base.GetTag()
			&& InTag.MatchesTag(Base.GetTag());
	}
}

USTRUCT(BlueprintType)
struct TOROSYSTEMS_API FToroCharacterEntry final : public FToroDatabaseEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Character)
	FText DisplayName;

	virtual bool IsValid() const override
	{
		return !DisplayName.IsEmptyOrWhitespace();
	}
};

UCLASS(NotBlueprintable, BlueprintType)
class TOROSYSTEMS_API UToroCharacterDatabase final : public UToroDatabase
{
	GENERATED_BODY()

public:

	UToroCharacterDatabase();
};
