// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "EquipmentHashTypes.h"
#include "FaerieSlotTag.h"

class UFaerieEquipmentHashAsset;
class UFaerieEquipmentManager;

class UFaerieItem;
using FEquipmentHashFunction = TFunctionRef<int32(const UFaerieItem*)>;

namespace Faerie::Hash
{
	// Get the hash of a FProperty's value on a specific object
	FAERIEEQUIPMENT_API int32 HashFProperty(const void* Ptr, const FProperty* Property);
	FAERIEEQUIPMENT_API int32 HashFProperty(const UObject* Obj, const FProperty* Property);

	FAERIEEQUIPMENT_API int32 HashStructByProps(const void* Ptr, const UScriptStruct* Struct, bool IncludeSuper);
	FAERIEEQUIPMENT_API int32 HashObjectByProps(const UObject* Obj, bool IncludeSuper);

	FAERIEEQUIPMENT_API FFaerieEquipmentHash MakeEquipmentHash(TConstArrayView<int32> Hashes);
	FAERIEEQUIPMENT_API FFaerieEquipmentHash HashItemSet(const TSet<const UFaerieItem*>& Items, const FEquipmentHashFunction& Function);
	FAERIEEQUIPMENT_API FFaerieEquipmentHash HashEquipment(const UFaerieEquipmentManager* Manager, const TSet<FFaerieSlotTag>& Slots, const FEquipmentHashFunction& Function);

	FAERIEEQUIPMENT_API bool ExecuteHashInstructions(const UFaerieEquipmentManager* Manager, const UFaerieEquipmentHashAsset* Asset);

	FAERIEEQUIPMENT_API int32 HashItemByName(const UFaerieItem* Item);
}