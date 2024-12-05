// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "Actions/GridExtensionClientActions.h"
#include "FaerieItemStorage.h"
#include "Extensions/InventoryGridExtensionBase.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GridExtensionClientActions)

bool FFaerieClientAction_RequestMoveItemBetweenGridSlots::Server_Execute(const UFaerieInventoryClient* Client) const
{
	if (!IsValid(Storage)) return false;
	if (!Client->CanAccessStorage(Storage)) return false;

	if (auto&& GridExtension = GetExtension<UInventoryGridExtensionBase>(Storage))
	{
		return GridExtension->MoveItem(TargetKey, DragEnd);
	}

	return false;
}

bool FFaerieClientAction_RequestRotateGridEntry::Server_Execute(const UFaerieInventoryClient* Client) const
{
	if (!IsValid(Storage)) return false;
	if (!Client->CanAccessStorage(Storage)) return false;

	if (auto&& GridExtension = GetExtension<UInventoryGridExtensionBase>(Storage))
	{
		return GridExtension->RotateItem(Key);
	}

	return false;
}