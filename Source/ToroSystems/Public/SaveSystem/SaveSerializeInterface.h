// Copyright (C) 2026 RedCraft86. LGPL-3.0-only, see LICENSE file for details.

#pragma once

#include "UObject/Interface.h"
#include "SaveSerializeInterface.generated.h"

UINTERFACE(MinimalAPI)
class USaveSerializeInterface : public UInterface
{
	GENERATED_BODY()
};

class TOROSYSTEMS_API ISaveSerializeInterface
{
	GENERATED_BODY()

protected:

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = SaveSystem)
	void PreSerialize(bool bLoading);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = SaveSystem)
	void PostSerialize(bool bLoading);

public:

	static void PreSerialize(UObject* Target, bool bLoading);
	static void PostSerialize(UObject* Target, bool bLoading);
};
