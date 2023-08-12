// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "Engine/DataAsset.h"
#include "DurabilityGradeAsset.generated.h"

/**
 *
 */
UCLASS(BlueprintType)
class FAERIEEQUIPMENT_API UDurabilityGradeAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Durability Grade")
	FText MaterialName;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Durability Grade")
	FText MaterialQuality;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Durability Grade")
	int32 Weaken;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Durability Grade")
	int32 Strength;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Durability Grade")
	float DefaultWeightPerCCM;
	//FWeightEditor_Float DefaultWeightPerCCM;
};