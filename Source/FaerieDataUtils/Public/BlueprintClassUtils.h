// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "BlueprintClassUtils.generated.h"

/**
 *
 */
UCLASS()
class FAERIEDATAUTILS_API UBlueprintClassUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Faerie|Utils", meta = (DeterminesOutputType = Class))
	static UObject* GetTypedOuter(UObject* Object, TSubclassOf<UObject> Class);
};