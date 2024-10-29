// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "CapacityStructs.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CapacityStructsLibrary.generated.h"

/**
 *
 */
UCLASS()
class UCapacityStructsUtilities : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Faerie|Inventory|Utils")
	static FItemCapacity WeightOfScaledComparison(const FItemCapacity& Original, const FItemCapacity& Comparison);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "WeightAndVolume + WeightAndVolume", CompactNodeTitle = "+", ScriptMethod = "Add",
		ScriptOperator = "+;+=", Keywords = "+ add plus", CommutativeAssociativeBinaryOperator = "true"), Category = "Faerie|Inventory|Utils")
	static FWeightAndVolume Add_WeightAndVolume(const FWeightAndVolume& A, const FWeightAndVolume& B) { return A + B; }

	UFUNCTION(BlueprintPure, meta = (DisplayName = "WeightAndVolume - WeightAndVolume", CompactNodeTitle = "-", ScriptMethod = "Subtract",
		ScriptOperator = "-;-=", Keywords = "- subtract minus", CommutativeAssociativeBinaryOperator = "true"), Category = "Faerie|Inventory|Utils")
	static FWeightAndVolume Subtract_WeightAndVolume(const FWeightAndVolume& A, const FWeightAndVolume& B) { return A - B; }

	UFUNCTION(BlueprintPure, Category = "Faerie|Inventory|Utils")
	static FWeightAndVolume ToWeightAndVolume_ItemCapacity(const FItemCapacity& ItemCapacity);
};