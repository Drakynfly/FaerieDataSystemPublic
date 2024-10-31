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
		Args.Get().Server_Execute(this);
	}
}

void FFaerieClientAction_RequestDeleteEntry::Server_Execute(const UFaerieInventoryClient* Client) const
{
	auto&& Storage = Handle.ItemStorage.Get();
	if (!IsValid(Storage)) return;
	if (!Client->CanAccessStorage(Storage)) return;

	Storage->RemoveStack(Handle.Key, FFaerieItemStorageEvents::Get().Removal_Deletion, Amount);
}

void FFaerieClientAction_RequestEjectEntry::Server_Execute(const UFaerieInventoryClient* Client) const
{
	auto&& Storage = Handle.ItemStorage.Get();
	if (!IsValid(Storage)) return;
	if (!Client->CanAccessStorage(Storage)) return;

	Storage->RemoveStack(Handle.Key, FFaerieEjectionEvent::Get().Removal_Ejection, Amount);
}

void FFaerieClientAction_RequestMoveEntry::Server_Execute(const UFaerieInventoryClient* Client) const
{
	auto&& Storage = Handle.ItemStorage.Get();
	if (!IsValid(Storage)) return;
	if (!IsValid(ToStorage)) return;
	if (!Client->CanAccessStorage(Storage)) return;
	if (Storage == ToStorage) return;

	Storage->MoveStack(ToStorage, Handle.Key, Amount);
}


void FFaerieClientAction_RequestSetItemInSlot::Server_Execute(const UFaerieInventoryClient* Client) const
{
	if (!IsValid(Slot)) return;
	if (!Client->CanAccessSlot(Slot)) return;
	if (!Slot->CanSetInSlot(Stack)) return;
	if (!IsValid(Stack.Item)) return;

	if (Slot->CanSetInSlot(Stack))
	{
		Slot->SetItemInSlot(Stack);
	}
}

void FFaerieClientAction_RequestMoveItemBetweenSlots::Server_Execute(const UFaerieInventoryClient* Client) const
{
	// Instant failure states
	if (!IsValid(FromSlot) ||
		!IsValid(ToSlot) ||
		!Client->CanAccessSlot(FromSlot) ||
		!Client->CanAccessSlot(ToSlot)) return;

	// If we are not in Swap mode, return if either FromSlot is *not* filled or ToSlot *is* filled.
	if (!CanSwapSlots &&
		(!FromSlot->IsFilled() || ToSlot->IsFilled()))
	{
		return;
	}

	// This must always pass.
	if (FromSlot->IsFilled() &&
		!ToSlot->CouldSetInSlot(FromSlot->View()))
	{
		return;
	}

	TOptional<FFaerieItemStack> ToSlotStack;
	if (ToSlot->IsFilled())
	{
		if (!FromSlot->CouldSetInSlot(ToSlot->View()))
		{
			// If we are in Swap mode, but the FromSlot cannot take the item in the ToSlot then we must abort
			return;
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
}


void FFaerieClientAction_RequestMoveEntryToEquipmentSlot::Server_Execute(const UFaerieInventoryClient* Client) const
{
	// @todo expose amount as a parameter
	int32 TempAmount = 1;

	auto&& Storage = Handle.ItemStorage.Get();
	if (!IsValid(Storage)) return;
	if (!Client->CanAccessStorage(Storage)) return;
	if (!IsValid(Slot)) return;
	if (!Client->CanAccessSlot(Slot)) return;
	if (!Slot->CanSetInSlot(Storage->View(Handle.Key.EntryKey).Resize(TempAmount))) return;

	FFaerieItemStack OutStack;
	if (!Storage->TakeStack(Handle.Key, OutStack, FFaerieItemStorageEvents::Get().Removal_Moving, TempAmount))
	{
		return;
	}

	if (IsValid(OutStack.Item) && OutStack.Copies == TempAmount)
	{
		Slot->SetItemInSlot(OutStack);
	}
}

void FFaerieClientAction_RequestMoveEquipmentSlotToInventory::Server_Execute(const UFaerieInventoryClient* Client) const
{
	if (!IsValid(Slot)) return;
	if (!Slot->IsFilled()) return;
	if (!Client->CanAccessSlot(Slot)) return;
	if (!IsValid(ToStorage)) return;
	if (!Client->CanAccessStorage(ToStorage)) return;

	int32 StackAmount = Amount;

	// We should verify that we can perform this move here first, before we call AddItemStack (even tho it does it too),
	// Otherwise we would have to remove the Item from the slot, and then add it back again if the Add failed :/
	if (StackAmount == Faerie::ItemData::UnlimitedStack)
	{
		StackAmount = Slot->GetCopies();
	}

	if (!ToStorage->CanAddStack({Slot->GetItemObject(), StackAmount}))
	{
		return;
	}

	if (const FFaerieItemStack Stack = Slot->TakeItemFromSlot(StackAmount);
		IsValid(Stack.Item))
	{
		ToStorage->AddItemStack(Stack);
	}
}

void FFaerieClientAction_RequestMoveItemBetweenSpatialSlots::Server_Execute(const UFaerieInventoryClient* Client) const
{
	//ToDo
}

void FFaerieClientAction_RequestRotateSpatialEntry::Server_Execute(const UFaerieInventoryClient* Client) const
{
	if (!IsValid(Storage)) return;
	if (!Client->CanAccessStorage(Storage)) return;

	if (auto&& SpatialExtension = Storage->GetExtension<UInventorySpatialGridExtension>())
	{
		// @todo the extension should know where the Loc is. we shouldn't have to provide it here!
		SpatialExtension->RotateItem(Key, Loc);
	}
}