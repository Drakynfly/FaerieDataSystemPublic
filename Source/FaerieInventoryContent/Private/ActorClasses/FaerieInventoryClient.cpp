// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "ActorClasses/FaerieInventoryClient.h"
#include "FaerieEquipmentSlot.h"
#include "FaerieItemStorage.h"
#include "Extensions/InventoryEjectionHandlerExtension.h"
#include "Extensions/InventorySpatialGridExtension.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FaerieInventoryClient)

UFaerieInventoryClient::UFaerieInventoryClient()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UFaerieInventoryClient::CanAccessContainer(const UFaerieItemContainerBase* Container) const
{
	// @todo implement
	/*
	if (auto&& PermissionExtensions = Storage->GetExtension<UInventoryClientPermissionExtensions>())
	{
		if (!PermissionExtensions->AllowsClient(this))
		{
			return false;
		}
	}
	*/
	return true;
}

bool UFaerieInventoryClient::CanAccessStorage(const UFaerieItemStorage* Storage) const
{
	return CanAccessContainer(Storage);
}

bool UFaerieInventoryClient::CanAccessEquipment(const UFaerieEquipmentManager* Equipment) const
{
	// @todo implement
	return true;
}

bool UFaerieInventoryClient::CanAccessSlot(const UFaerieEquipmentSlot* Slot) const
{
	return CanAccessContainer(Slot) /** && CanAccessEquipment(Slot->GetOuterManager())*/;
}

void UFaerieInventoryClient::RequestExecuteAction_Implementation(const TInstancedStruct<FFaerieClientActionBase>& Args)
{
	if (Args.IsValid())
	{
		(void)Args.Get().Server_Execute(this);
	}
}

void UFaerieInventoryClient::RequestExecuteAction_Batch_Implementation(
	const TArray<TInstancedStruct<FFaerieClientActionBase>>& Args, const EFaerieClientRequestBatchType Type)
{
	for (auto&& Element : Args)
	{
		bool Ran = false;
		if (Element.IsValid())
		{
			Ran = Element.Get().Server_Execute(this);
		}

		if (!Ran && Type == EFaerieClientRequestBatchType::Sequence)
		{
			// Sequence failed, exit.
			return;
		}
	}
}

bool FFaerieClientAction_RequestDeleteEntry::Server_Execute(const UFaerieInventoryClient* Client) const
{
	auto&& Storage = Handle.ItemStorage.Get();
	if (!IsValid(Storage)) return false;
	if (!Client->CanAccessStorage(Storage)) return false;

	return Storage->RemoveStack(Handle.Key, Faerie::Inventory::Tags::RemovalDeletion, Amount);
}

bool FFaerieClientAction_RequestEjectEntry::Server_Execute(const UFaerieInventoryClient* Client) const
{
	auto&& Storage = Handle.ItemStorage.Get();
	if (!IsValid(Storage)) return false;
	if (!Client->CanAccessStorage(Storage)) return false;

	return Storage->RemoveStack(Handle.Key, Faerie::Inventory::Tags::RemovalEject, Amount);
}

bool FFaerieClientAction_RequestMoveEntry::Server_Execute(const UFaerieInventoryClient* Client) const
{
	auto&& Storage = Handle.ItemStorage.Get();
	if (!IsValid(Storage)) return false;
	if (!IsValid(ToStorage)) return false;
	if (!Client->CanAccessStorage(Storage)) return false;
	if (Storage == ToStorage) return false;

	return Storage->MoveStack(ToStorage, Handle.Key, Amount).IsValid();
}


bool FFaerieClientAction_RequestSetItemInSlot::Server_Execute(const UFaerieInventoryClient* Client) const
{
	if (!IsValid(Slot)) return false;
	if (!Client->CanAccessSlot(Slot)) return false;
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
		!Client->CanAccessSlot(FromSlot) ||
		!Client->CanAccessSlot(ToSlot)) return false;

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
	// @todo expose amount as a parameter
	int32 TempAmount = 1;

	auto&& Storage = Handle.ItemStorage.Get();
	if (!IsValid(Storage)) return false;
	if (!Client->CanAccessStorage(Storage)) return false;
	if (!IsValid(Slot)) return false;
	if (!Client->CanAccessSlot(Slot)) return false;
	if (!Slot->CanSetInSlot(Storage->View(Handle.Key.EntryKey).Resize(TempAmount))) return false;

	FFaerieItemStack OutStack;
	if (!Storage->TakeStack(Handle.Key, OutStack, Faerie::Inventory::Tags::RemovalMoving, TempAmount))
	{
		return false;
	}

	if (IsValid(OutStack.Item) && OutStack.Copies == TempAmount)
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
	if (!Client->CanAccessSlot(Slot)) return false;
	if (!IsValid(ToStorage)) return false;
	if (!Client->CanAccessStorage(ToStorage)) return false;

	int32 StackAmount = Amount;

	// We should verify that we can perform this move here first, before we call AddItemStack (even tho it does it too),
	// Otherwise we would have to remove the Item from the slot, and then add it back again if the Add failed :/
	if (StackAmount == Faerie::ItemData::UnlimitedStack)
	{
		StackAmount = Slot->GetCopies();
	}

	if (!ToStorage->CanAddStack({Slot->GetItemObject(), StackAmount}))
	{
		return false;
	}

	if (const FFaerieItemStack Stack = Slot->TakeItemFromSlot(StackAmount);
		IsValid(Stack.Item))
	{
		return ToStorage->AddItemStack(Stack);
	}

	return false;
}

bool FFaerieClientAction_RequestMoveItemBetweenSpatialSlots::Server_Execute(const UFaerieInventoryClient* Client) const
{
	if (!IsValid(Storage)) return false;
	if (!Client->CanAccessStorage(Storage)) return false;

	if (auto&& SpatialExtension = Storage->GetExtension<UInventorySpatialGridExtension>())
	{
		return SpatialExtension->MoveItem(TargetKey, DragEnd);
	}

	return false;
}

bool FFaerieClientAction_RequestRotateSpatialEntry::Server_Execute(const UFaerieInventoryClient* Client) const
{
	if (!IsValid(Storage)) return false;
	if (!Client->CanAccessStorage(Storage)) return false;

	if (auto&& SpatialExtension = Storage->GetExtension<UInventorySpatialGridExtension>())
	{
		return SpatialExtension->RotateItem(Key);
	}

	return false;
}