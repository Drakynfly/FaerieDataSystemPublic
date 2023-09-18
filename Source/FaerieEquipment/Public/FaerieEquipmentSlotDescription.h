// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "Engine/DataAsset.h"
#include "FaerieEquipmentSlotDescription.generated.h"

class UFaerieItemTemplate;

/**
 * @todo If it's determined that this class doesn't need to store anything except the ItemTemplate, then deprecate this,
 * and just use ItemTemplate instead
 */
UCLASS()
class FAERIEEQUIPMENT_API UFaerieEquipmentSlotDescription : public UDataAsset
{
	GENERATED_BODY()

public:
	// Items must match this template in order to fill the slot.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "EquipmentSlotDescription")
	TObjectPtr<UFaerieItemTemplate> Template;
};