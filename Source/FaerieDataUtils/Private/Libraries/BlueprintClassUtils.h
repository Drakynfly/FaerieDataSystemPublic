// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "BlueprintClassUtils.generated.h"

/**
 *
 */
UCLASS()
class UBlueprintClassUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// Traverse the outer chain to find the first Outer of a given class (or nullptr if there isn't one).
	UFUNCTION(BlueprintCallable, Category = "Faerie|Utils", meta = (DeterminesOutputType = Class))
	static UObject* GetTypedOuter(UObject* Object, TSubclassOf<UObject> Class);

	// Traverse the attached parent chain to find the first Parent of a given class (or nullptr if there isn't one).
	UFUNCTION(BlueprintCallable, Category = "Faerie|Utils", meta = (DeterminesOutputType = Class))
	static USceneComponent* GetTypedParent(USceneComponent* Component, TSubclassOf<USceneComponent> Class);
};