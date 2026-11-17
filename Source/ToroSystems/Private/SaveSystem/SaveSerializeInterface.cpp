// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#include "SaveSystem/SaveSerializeInterface.h"
#include "Utilities/ToroUtilities.h"

void ISaveSerializeInterface::PreSerialize(UObject* Target, bool bLoading)
{
	if (ToroEngine::ImplementsInterface<USaveSerializeInterface>(Target))
	{
		Execute_PreSerialize(Target, bLoading);
	}
}

void ISaveSerializeInterface::PostSerialize(UObject* Target, bool bLoading)
{
	if (ToroEngine::ImplementsInterface<USaveSerializeInterface>(Target))
	{
		Execute_PostSerialize(Target, bLoading);
	}
}
