// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "EquipmentQueryLibrary.h"
#include "EquipmentQueryStatics.h"

bool UFaerieEquipmentQueryLibrary::RunEquipmentQuery(const UFaerieEquipmentManager* Manager, const FFaerieEquipmentSetQuery& SetQuery, UFaerieEquipmentSlot*& PassingSlot)
{
	return Faerie::Equipment::RunEquipmentQuery(Manager, SetQuery, PassingSlot);
}