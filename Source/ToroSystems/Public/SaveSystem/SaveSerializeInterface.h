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
	void OnPreSerialize(bool bIsLoading);
	virtual void OnPreSerialize_Implementation(bool bIsLoading) {}

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = SaveSystem)
	void OnPostSerialize(bool bIsLoading);
	virtual void OnPostSerialize_Implementation(bool bIsLoading) {}

public:

	static void PreSerialize(UObject* Target, bool bIsLoading);
	static void PostSerialize(UObject* Target, bool bIsLoading);
};
