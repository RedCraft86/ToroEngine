// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#include "Objects/ToroDataAsset.h"
#if WITH_EDITOR
#include "Utilities/ToroValidation.h"
#endif

UToroDataAsset::UToroDataAsset()
{
}

#if WITH_EDITOR
void UToroDataAsset::ValidateData() const
{
	ToroEngine::Validation::ValidateAsset(this);
}

EDataValidationResult UToroDataAsset::IsDataValid(FDataValidationContext& Context) const
{
	return Super::IsDataValid(Context);
}
#endif