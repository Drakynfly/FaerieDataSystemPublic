// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "FaerieHash.h"
#include "FaerieHashStatics.h"
#include "FaerieSlotTag.h"

class UFaerieEquipmentManager;
class UFaerieEquipmentHashAsset;

namespace Faerie::Hash
{
	FAERIEEQUIPMENT_API FFaerieHash HashEquipment(const UFaerieEquipmentManager* Manager, const TSet<FFaerieSlotTag>& Slots, const FFaerieItemHashFunction& Function);

	FAERIEEQUIPMENT_API bool ExecuteHashInstructions(const UFaerieEquipmentManager* Manager, const UFaerieEquipmentHashAsset* Asset);
}