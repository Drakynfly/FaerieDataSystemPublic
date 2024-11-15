// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "FaerieSlotTag.h"
#include "FaerieEquipmentSlotConfig.generated.h"

class UFaerieEquipmentSlotDescription;

USTRUCT(BlueprintType)
struct FFaerieEquipmentSlotConfig
{
	GENERATED_BODY()

	// Unique ID for this slot.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	FFaerieSlotTag SlotID;

	// Info about this slot.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	TObjectPtr<UFaerieEquipmentSlotDescription> SlotDescription;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	bool SingleItemSlot;
};