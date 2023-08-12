// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "FaerieInfoObject.h"
#include "Engine/DataAsset.h"
#include "FaerieEquipmentSlotDescription.generated.h"

class UFaerieItemTemplate;

/**
 *
 */
UCLASS()
class FAERIEEQUIPMENT_API UFaerieEquipmentSlotDescription : public UDataAsset
{
	GENERATED_BODY()

public:
	// Info about this slot.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "EquipmentSlotDescription")
	FFaerieAssetInfo SlotInfo;

	// Items must match this template in order to fill the slot.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "EquipmentSlotDescription")
	TObjectPtr<UFaerieItemTemplate> Template;
};