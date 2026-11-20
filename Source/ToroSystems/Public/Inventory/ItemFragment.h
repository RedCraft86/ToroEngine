// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "CoreMinimal.h"
#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif
#include "ItemFragment.generated.h"

class UTexture2D;
class AEquipmentActor;

USTRUCT(BlueprintInternalUseOnly)
struct FItemFragment
{
	GENERATED_BODY()

	virtual ~FItemFragment() = default;

	uint8 EnforceLimit = 0;
	float PriorityOffset = 0.0f;

#if WITH_EDITOR
	virtual bool Validate(FDataValidationContext& Context) const
	{
		Context.AddError(INVTEXT("Invalid fragment selection."));
		return false;
	}
#endif
};

USTRUCT(BlueprintInternalUseOnly)
struct FItemTextFragment final : public FItemFragment
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = Archive)
	FText Title;

	UPROPERTY(EditAnywhere, Category = Archive, meta = (MultiLine = true))
	FText BodyText;

	UPROPERTY(EditAnywhere, Category = Archive)
	TSoftObjectPtr<UTexture2D> ContentImage;

	FItemTextFragment()
	{
		EnforceLimit = 1;
		PriorityOffset = +2.0f;
	}

#if WITH_EDITOR
	virtual bool Validate(FDataValidationContext& Context) const override
	{
		bool bValid = true;
		if (Title.IsEmptyOrWhitespace())
		{
			Context.AddError(INVTEXT("Fragment missing title or it is whitespace."));
			bValid = false;
		}
		if (BodyText.IsEmptyOrWhitespace() && ContentImage.IsNull())
		{
			Context.AddError(INVTEXT("Fragment needs at least body text or content image."));
			bValid = false;
		}
		return bValid;
	}
#endif
};

USTRUCT(BlueprintInternalUseOnly)
struct FItemEquipmentFragment final : public FItemFragment
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = Equipment, NoClear, meta = (AllowAbstract = false))
	TSoftClassPtr<AEquipmentActor> EquipmentClass;

	FItemEquipmentFragment()
	{
		EnforceLimit = 1;
		PriorityOffset = -5.0f;
	}

#if WITH_EDITOR
	virtual bool Validate(FDataValidationContext& Context) const override
	{
		if (!EquipmentClass.IsValid() || EquipmentClass.LoadSynchronous()->HasAnyClassFlags(CLASS_Abstract))
		{
			Context.AddError(INVTEXT("Fragment requires valid equipment class."));
			return false;
		}
		return true;
	}
#endif
};