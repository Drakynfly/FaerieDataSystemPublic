// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "ActorClasses/FaerieInventoryClient.h"
#include "FaerieEquipmentSlot.h"
#include "FaerieItemStorage.h"
#include "Extensions/InventoryEjectionHandlerExtension.h"
#include "Extensions/InventoryUserdataExtension.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FaerieInventoryClient)

UFaerieInventoryClient::UFaerieInventoryClient()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UFaerieInventoryClient::CanAccessContainer(UFaerieItemContainerBase* Container) const
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

bool UFaerieInventoryClient::CanAccessStorage(UFaerieItemStorage* Storage) const
{
	return CanAccessContainer(Storage);
}

bool UFaerieInventoryClient::CanAccessEquipment(UFaerieEquipmentManager* Equipment) const
{
	// @todo implement
	return true;
}

bool UFaerieInventoryClient::CanAccessSlot(UFaerieEquipmentSlot* Slot) const
{
	return CanAccessContainer(Slot) /** && CanAccessEquipment(Slot->GetOuterManager())*/;
}

void UFaerieInventoryClient::RequestDeleteEntry_Implementation(const FInventoryKeyHandle Handle, const int32 Amount)
{
	auto&& Storage = Handle.ItemStorage.Get();
	if (!IsValid(Storage)) return;
	if (!CanAccessStorage(Storage)) return;

	Storage->RemoveStack(Handle.Key, FFaerieItemStorageEvents::Get().Removal_Deletion, Amount);
}

void UFaerieInventoryClient::RequestEjectEntry_Implementation(const FInventoryKeyHandle Handle, const int32 Amount)
{
	auto&& Storage = Handle.ItemStorage.Get();
	if (!IsValid(Storage)) return;
	if (!CanAccessStorage(Storage)) return;

	Storage->RemoveStack(Handle.Key, FFaerieEjectionEvent::Get().Removal_Ejection, Amount);
}

void UFaerieInventoryClient::RequestMoveEntry_Implementation(const FInventoryKeyHandle Handle, const int32 Amount,
															 UFaerieItemStorage* ToStorage)
{
	auto&& Storage = Handle.ItemStorage.Get();
	if (!IsValid(Storage)) return;
	if (!IsValid(ToStorage)) return;
	if (!CanAccessStorage(Storage)) return;
	if (Storage == ToStorage) return;

	Storage->MoveStack(ToStorage, Handle.Key, Amount);
}

void UFaerieInventoryClient::RequestMarkStackWithTag_Implementation(const FInventoryKeyHandle Handle,
                                                                    const FFaerieInventoryUserTag Tag)
{
	auto&& Storage = Handle.ItemStorage.Get();
	if (!IsValid(Storage)) return;
	if (!CanAccessStorage(Storage)) return;

	if (auto&& Userdata = Storage->GetExtension<UInventoryUserdataExtension>())
	{
		Userdata->MarkStackWithTag(Handle.ItemStorage.Get(), Handle.Key.EntryKey, Tag);
	}
}

void UFaerieInventoryClient::RequestClearTagFromStack_Implementation(const FInventoryKeyHandle Handle,
                                                                     const FFaerieInventoryUserTag Tag)
{
	auto&& Storage = Handle.ItemStorage.Get();
	if (!IsValid(Storage)) return;
	if (!CanAccessStorage(Storage)) return;

	if (auto&& Userdata = Storage->GetExtension<UInventoryUserdataExtension>())
	{
		Userdata->ClearTagFromStack(Handle.ItemStorage.Get(), Handle.Key.EntryKey, Tag);
	}
}

void UFaerieInventoryClient::RequestSetItemInSlot_Implementation(UFaerieEquipmentSlot* Slot, const FFaerieItemStack Stack)
{
	if (!IsValid(Slot)) return;
	if (!CanAccessSlot(Slot)) return;
	if (!Slot->CanSetInSlot(Stack)) return;
	if (!IsValid(Stack.Item)) return;

	if (Slot->CanSetInSlot(Stack))
	{
		Slot->SetItemInSlot(Stack);
	}
}

void UFaerieInventoryClient::RequestMoveItemBetweenSlots_Implementation(UFaerieEquipmentSlot* FromSlot,
                                                                        UFaerieEquipmentSlot* ToSlot,
                                                                        const bool CanSwapSlots)
{
	// Instant failure states
	if (!IsValid(FromSlot) ||
		!IsValid(ToSlot) ||
		!CanAccessSlot(FromSlot) ||
		!CanAccessSlot(ToSlot)) return;

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


void UFaerieInventoryClient::RequestMoveEntryToEquipmentSlot_Implementation(const FInventoryKeyHandle Handle,
																			UFaerieEquipmentSlot* Slot)
{
	// @todo expose amount as a parameter
	int32 TempAmount = 1;

	auto&& Storage = Handle.ItemStorage.Get();
	if (!IsValid(Storage)) return;
	if (!CanAccessStorage(Storage)) return;
	if (!IsValid(Slot)) return;
	if (!CanAccessSlot(Slot)) return;
	if (!Slot->CanSetInSlot({Storage->View(Handle.Key.EntryKey).Item, TempAmount})) return;

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

void UFaerieInventoryClient::RequestMoveEquipmentSlotToInventory_Implementation(UFaerieEquipmentSlot* Slot,
																				UFaerieItemStorage* ToStorage,
																				int32 Amount)
{
	if (!IsValid(Slot)) return;
	if (!Slot->IsFilled()) return;
	if (!CanAccessSlot(Slot)) return;
	if (!IsValid(ToStorage)) return;
	if (!CanAccessStorage(ToStorage)) return;

	// We should verify that we can perform this move here first, before we call AddItemStack (event tho it does it too),
	// Otherwise we would have to remove the Item from the slot, and then add it back again if the Add failed :/
	if (Amount == Faerie::ItemData::UnlimitedStack)
	{
		Amount = Slot->GetCopies();
	}
	if (!ToStorage->CanAddStack({Slot->GetItemObject(), Amount})) return;

	if (const FFaerieItemStack Stack = Slot->TakeItemFromSlot(Amount);
		IsValid(Stack.Item))
	{
		ToStorage->AddItemStack(Stack);
	}
}