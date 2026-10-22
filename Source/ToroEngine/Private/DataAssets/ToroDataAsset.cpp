// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "DataAssets/ToroDataAsset.h"

#if WITH_EDITOR
void UToroDataAsset::RefreshData()
{
	// Not implemented in base class
}

EDataValidationResult UToroDataAsset::IsDataValid(FDataValidationContext& Context) const
{
	return EDataValidationResult::Valid;
}
#endif
