// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

struct FFaerieEquipmentSetQuery;
class UFaerieEquipmentManager;
class UFaerieEquipmentSlot;

namespace Faerie::Equipment
{
	bool RunEquipmentQuery(const UFaerieEquipmentManager* Manager, const FFaerieEquipmentSetQuery& SetQuery, UFaerieEquipmentSlot*& PassingSlot);
}