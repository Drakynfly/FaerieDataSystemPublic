// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "FaerieHash.h"
#include "FaerieSlotTag.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "EquipmentHashLibrary.generated.h"

class UFaerieEquipmentHashAsset;
class UFaerieEquipmentManager;

class UFaerieItem;
DECLARE_DYNAMIC_DELEGATE_RetVal_OneParam(int32, FBlueprintEquipmentHash, const UFaerieItem*, Item);

USTRUCT(BlueprintType)
struct FFaerieEquipmentHashConfig
{
	GENERATED_BODY()

	// Which slots to use in the hash
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EquipmentHashConfig")
	TSet<FFaerieSlotTag> Slots;

	// Hash function
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EquipmentHashConfig")
	FBlueprintEquipmentHash HashFunction;
};

UCLASS()
class UFaerieEquipmentHashLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Faerie|EquipmentHash")
	static int64 BreakEquipmentHash(const FFaerieHash Hash) { return Hash.Hash; }

	// Generate a hash from a set of slots. Typically used for checksum'ing.
	UFUNCTION(BlueprintCallable, Category = "Faerie|EquipmentHashing")
	static FFaerieHash HashEquipment(const UFaerieEquipmentManager* Manager, const FFaerieEquipmentHashConfig& Config);

	// Generate a hash from a set of slots, using a predefined asset.
	UFUNCTION(BlueprintCallable, Category = "Faerie|EquipmentHashing")
	static bool ExecuteHashInstructions(const UFaerieEquipmentManager* Manager, const UFaerieEquipmentHashAsset* Asset);

	UFUNCTION(BlueprintPure, Category = "Faerie|EquipmentHashing")
	static FBlueprintEquipmentHash GetEquipmentHash_ByName();

protected:
	static int32 ExecHashItemByName(const UFaerieItem* Item);
};