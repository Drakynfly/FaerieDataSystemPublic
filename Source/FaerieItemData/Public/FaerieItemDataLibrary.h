// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "FaerieItemDataLibrary.generated.h"

class UFaerieItem;
class UFaerieItemToken;

/**
 *
 */
UCLASS()
class FAERIEITEMDATA_API UFaerieItemDataLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Faerie|ItemDataLibrary")
	static bool Equal_ItemData(const UFaerieItem* A, const UFaerieItem* B);

	UFUNCTION(BlueprintPure, Category = "Faerie|ItemDataLibrary")
	static bool Equal_ItemToken(const UFaerieItemToken* A, const UFaerieItemToken* B);
};
