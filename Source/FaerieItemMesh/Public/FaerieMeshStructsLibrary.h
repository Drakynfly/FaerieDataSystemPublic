// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include "FaerieMeshStructsLibrary.generated.h"

/**
 *
 */
UCLASS()
class FAERIEITEMMESH_API UFaerieMeshStructsLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Inventory|Utils")
	static TArray<UMaterialInterface*> FaerieItemMaterialsToObjectArray(const TArray<FFaerieItemMaterial>& Materials);
};
