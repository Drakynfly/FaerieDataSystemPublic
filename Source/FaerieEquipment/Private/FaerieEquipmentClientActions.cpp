// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "FaerieEquipmentClientActions.h"
#include "FaerieEquipmentSlot.h"
#include "FaerieItemStorage.h"
#include "Extensions/InventoryGridExtensionBase.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FaerieEquipmentClientActions)

bool FFaerieClientAction_RequestSetItemInSlot::Server_Execute(const UFaerieInventoryClient* Client) const
{
	if (!IsValid(Slot)) return false;
	if (!Slot->CanClientRunActions(Client)) return false;
	if (!Slot->CanSetInSlot(Stack)) return false;
	if (!IsValid(Stack.Item)) return false;

	if (Slot->CanSetInSlot(Stack))
	{
		Slot->SetItemInSlot(Stack);
		return true;
	}
	return false;
}

bool FFaerieClientAction_RequestMoveItemBetweenSlots::Server_Execute(const UFaerieInventoryClient* Client) const
{
	// Instant failure states
	if (!IsValid(FromSlot) ||
		!IsValid(ToSlot) ||
		!FromSlot->CanClientRunActions(Client) ||
		!ToSlot->CanClientRunActions(Client)) return false;

	// If we are not in Swap mode, return if either FromSlot is *not* filled or ToSlot *is* filled.
	if (!CanSwapSlots &&
		(!FromSlot->IsFilled() || ToSlot->IsFilled()))
	{
		return false;
	}

	// This must always pass.
	if (FromSlot->IsFilled() &&
		!ToSlot->CouldSetInSlot(FromSlot->View()))
	{
		return false;
	}

	TOptional<FFaerieItemStack> ToSlotStack;
	if (ToSlot->IsFilled())
	{
		if (!FromSlot->CouldSetInSlot(ToSlot->View()))
		{
			// If we are in Swap mode, but the FromSlot cannot take the item in the ToSlot then we must abort
			return false;
		}

		ToSlotStack = ToSlot->TakeItemFromSlot(Faerie::ItemData::UnlimitedStack);
	}

	if (const FFaerieItemStack FromSlotStack = FromSlot->TakeItemFromSlot(Faerie::ItemData::UnlimitedStack);
		IsValid(FromSlotStack.Item))
	{
		ToSlot->SetItemInSlot(FromSlotStack);
	}

	if (ToSlotStack.IsSet())
	{
		FromSlot->SetItemInSlot(ToSlotStack.GetValue());
	}

	return true;
}

bool FFaerieClientAction_RequestMoveEntryToEquipmentSlot::Server_Execute(const UFaerieInventoryClient* Client) const
{
	auto&& Storage = Handle.ItemStorage.Get();
	if (!IsValid(Storage)) return false;
	if (!Client->CanAccessStorage(Storage)) return false;
	if (!IsValid(Slot)) return false;
	if (!Slot->CanClientRunActions(Client)) return false;
	if (!Slot->CanSetInSlot(Storage->View(Handle.Key.EntryKey).Resize(Amount))) return false;

	FFaerieItemStack OutStack;
	if (!Storage->TakeStack(Handle.Key, OutStack, Faerie::Inventory::Tags::RemovalMoving, Amount))
	{
		return false;
	}

	if (IsValid(OutStack.Item) && OutStack.Copies == Amount)
	{
		Slot->SetItemInSlot(OutStack);
		return true;
	}

	return false;
}

bool FFaerieClientAction_RequestMoveEquipmentSlotToInventory::Server_Execute(const UFaerieInventoryClient* Client) const
{
	if (!IsValid(Slot)) return false;
	if (!Slot->IsFilled()) return false;
	if (!Slot->CanClientRunActions(Client)) return false;
	if (!IsValid(ToStorage)) return false;
	if (!Client->CanAccessStorage(ToStorage)) return false;

	int32 StackAmount = Amount;

	// We should verify that we can perform this move here first, before we call AddItemStack (even tho it does it too),
	// Otherwise we would have to remove the Item from the slot, and then add it back again if the Add failed :/
	if (StackAmount == Faerie::ItemData::UnlimitedStack)
	{
		StackAmount = Slot->GetCopies();
	}

	if (!ToStorage->CanAddStack({Slot->GetItemObject(), StackAmount}, AddStackBehavior))
	{
		return false;
	}

	if (const FFaerieItemStack Stack = Slot->TakeItemFromSlot(StackAmount);
		IsValid(Stack.Item))
	{
		return ToStorage->AddItemStack(Stack, AddStackBehavior);
	}

	return false;
}

bool FFaerieClientAction_RequestMoveEquipmentSlotToSpatialInventory::Server_Execute(const UFaerieInventoryClient* Client) const
{
	// Validate Slot and Storage access
	if (!IsValid(Slot) ||
		!Slot->IsFilled() ||
		TargetPoint == FIntPoint::NoneValue ||
		!Slot->CanClientRunActions(Client) ||
		!IsValid(ToStorage) ||
		!Client->CanAccessStorage(ToStorage))
	{
		return false;
	}

	// Fetch the Grid Extension and ensure it exists
	auto&& GridExtension = GetExtension<UInventoryGridExtensionBase>(ToStorage);
	if (!IsValid(GridExtension))
	{
		return false;
	}

	if (!GridExtension->CanAddAtLocation(Slot->View(), TargetPoint))
	{
		return false;
	}

	// Determine the stack amount to transfer
	const int32 StackAmount = Amount == Faerie::ItemData::UnlimitedStack ? Slot->GetCopies() : Amount;

	// Attempt to transfer the item stack
	if (const FFaerieItemStack Stack = Slot->TakeItemFromSlot(StackAmount);
		IsValid(Stack.Item))
	{
		// Must be a new stack, since we intend to manually place it in the grid.
		const FLoggedInventoryEvent Event = ToStorage->AddItemStackWithLog(Stack, EFaerieStorageAddStackBehavior::OnlyNewStacks);
		if (!Event.Event.Success)
		{
			return false;
		}

		const FInventoryKey TargetKey(Event.Event.EntryTouched, Event.Event.StackKeys.Last());

		// Finally, move item to the cell client requested.
		return GridExtension->MoveItem(TargetKey, TargetPoint);
	}

	return false;
}