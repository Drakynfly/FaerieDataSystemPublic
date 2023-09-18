// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "EquipmentHashTypes.generated.h"

USTRUCT(BlueprintType, meta = (HasNativeBreak = "/Script/FaerieEquipment.FaerieEquipmentHashLibrary.BreakEquipmentHash"))
struct FFaerieEquipmentHash
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "EquipmentHash")
	uint32 Hash = 0;

	friend bool operator==(const FFaerieEquipmentHash& Lhs, const FFaerieEquipmentHash& Rhs)
	{
		return Lhs.Hash == Rhs.Hash;
	}

	friend bool operator!=(const FFaerieEquipmentHash& Lhs, const FFaerieEquipmentHash& Rhs) { return !(Lhs == Rhs); }
};

UCLASS()
class UFaerieEquipmentHashLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Faerie|EquipmentHash")
	static int64 BreakEquipmentHash(const FFaerieEquipmentHash Hash) { return Hash.Hash; }
};