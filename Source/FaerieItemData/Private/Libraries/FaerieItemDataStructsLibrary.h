// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "FaerieItemDataStructsLibrary.generated.h"

/**
 *
 */
UCLASS()
class UFaerieItemDataStructsLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// Get the inventory system Unlimited Stack.
	UFUNCTION(BlueprintPure, Category = "Faerie|ItemData|Macros")
	static int32 UnlimitedStack();

	// Tests if a stack is equal to Unlimited Stack.
	UFUNCTION(BlueprintPure, Category = "Faerie|ItemData|Macros")
	static bool IsUnlimited(int32 Stack);
};