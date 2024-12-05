// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "Actions/GridExtensionClientActions.h"
#include "FaerieItemStorage.h"
#include "Extensions/InventorySpatialGridExtension.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GridExtensionClientActions)

bool FFaerieClientAction_RequestMoveItemBetweenSpatialSlots::Server_Execute(const UFaerieInventoryClient* Client) const
{
	if (!IsValid(Storage)) return false;
	if (!Client->CanAccessStorage(Storage)) return false;

	if (auto&& SpatialExtension = GetExtension<UInventoryGridExtensionBase>(Storage))
	{
		return SpatialExtension->MoveItem(TargetKey, DragEnd);
	}

	return false;
}

bool FFaerieClientAction_RequestRotateSpatialEntry::Server_Execute(const UFaerieInventoryClient* Client) const
{
	if (!IsValid(Storage)) return false;
	if (!Client->CanAccessStorage(Storage)) return false;

	if (auto&& SpatialExtension = GetExtension<UInventoryGridExtensionBase>(Storage))
	{
		return SpatialExtension->RotateItem(Key);
	}

	return false;
}