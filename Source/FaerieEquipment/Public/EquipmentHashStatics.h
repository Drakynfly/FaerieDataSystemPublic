// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "FaerieHash.h"
#include "FaerieSlotTag.h"

class UFaerieEquipmentHashAsset;
class UFaerieEquipmentManager;
class UFaerieItem;
class UFaerieItemContainerBase;

using FFaerieItemHashFunction = TFunctionRef<uint32(const UFaerieItem*)>;

namespace Faerie::Hash
{
	FAERIEEQUIPMENT_API FFaerieHash CombineHashes(TArray<uint32>& Hashes);

	// Get the hash of a FProperty's value on a specific object
	FAERIEEQUIPMENT_API uint32 HashFProperty(const void* Ptr, const FProperty* Property);

	FAERIEEQUIPMENT_API uint32 HashStructByProps(const void* Ptr, const UScriptStruct* Struct, bool IncludeSuper);
	FAERIEEQUIPMENT_API uint32 HashObjectByProps(const UObject* Obj, bool IncludeSuper);

	FAERIEEQUIPMENT_API FFaerieHash HashItemSet(const TSet<const UFaerieItem*>& Items, const FFaerieItemHashFunction& Function);
	FAERIEEQUIPMENT_API FFaerieHash HashContainer(const UFaerieItemContainerBase* Container, const FFaerieItemHashFunction& Function);
	FAERIEEQUIPMENT_API FFaerieHash HashContainers(const TConstArrayView<const UFaerieItemContainerBase*> Containers, const FFaerieItemHashFunction& Function);
	FAERIEEQUIPMENT_API FFaerieHash HashEquipment(const UFaerieEquipmentManager* Manager, const TSet<FFaerieSlotTag>& Slots, const FFaerieItemHashFunction& Function);

	FAERIEEQUIPMENT_API bool ExecuteHashInstructions(const UFaerieEquipmentManager* Manager, const UFaerieEquipmentHashAsset* Asset);

	FAERIEEQUIPMENT_API uint32 HashItemByName(const UFaerieItem* Item);
}