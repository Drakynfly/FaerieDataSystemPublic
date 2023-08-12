// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "ActorClasses/FaerieInventoryClient.h"
#include "FaerieEquipmentManager.h"
#include "FaerieEquipmentSlot.h"
#include "FaerieItemStorage.h"
#include "Extensions/InventoryEjectionHandlerExtension.h"
#include "Extensions/InventoryUserdataExtension.h"

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
	return CanAccessContainer(Slot) && CanAccessEquipment(Slot->GetOuterManager());
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

void UFaerieInventoryClient::RequestSetItemInSlot_Implementation(UFaerieEquipmentSlot* Slot, UFaerieItem* Item)
{
	if (!IsValid(Slot)) return;
	if (!CanAccessSlot(Slot)) return;
	if (!Slot->IsFilled()) return;
	if (!IsValid(Item)) return;

	if (Slot->CanSetInSlot(Item))
	{
		Slot->SetItemInSlot(Item);
	}
}

void UFaerieInventoryClient::RequestMoveItemBetweenSlots_Implementation(UFaerieEquipmentSlot* FromSlot,
                                                                        UFaerieEquipmentSlot* ToSlot,
                                                                        const bool CanSwapSlots)
{
	if (!IsValid(FromSlot)) return;
	if (!IsValid(ToSlot)) return;
	if (!CanAccessSlot(FromSlot)) return;
	if (!CanAccessSlot(ToSlot)) return;
	if (!FromSlot->IsFilled()) return;
	if (ToSlot->IsFilled() && !CanSwapSlots) return;

	if (!ToSlot->CanSetInSlot(FromSlot->GetItem()))
	{
		return;
	}

	UFaerieItem* ToSlotItem = nullptr;
	if (ToSlot->IsFilled() && CanSwapSlots)
	{
		// If we have to make a swap, but the FromSlot cannot take the item in the ToSlot then we must abort
		if (!FromSlot->CanSetInSlot(ToSlot->GetItem()))
		{
			return;
		}

		ToSlotItem = ToSlot->TakeItemFromSlot();
	}

	UFaerieItem* Item = FromSlot->TakeItemFromSlot();
	ToSlot->SetItemInSlot(Item);

	if (IsValid(ToSlotItem))
	{
		FromSlot->SetItemInSlot(ToSlotItem);
	}
}


void UFaerieInventoryClient::RequestMoveEntryToEquipmentSlot_Implementation(const FInventoryKeyHandle Handle,
																			UFaerieEquipmentSlot* Slot)
{
	auto&& Storage = Handle.ItemStorage.Get();
	if (!IsValid(Storage)) return;
	if (!CanAccessStorage(Storage)) return;
	if (!IsValid(Slot)) return;
	if (Slot->IsFilled()) return;
	if (!CanAccessSlot(Slot)) return;

	FFaerieItemStack OutStack;
	if (!Storage->TakeStack(Handle.Key, OutStack, FFaerieItemStorageEvents::Get().Removal_Moving, 1))
	{
		return;
	}

	if (IsValid(OutStack.Item) && OutStack.Copies == 1)
	{
		Slot->SetItemInSlot(OutStack.Item);
	}
}

void UFaerieInventoryClient::RequestMoveEquipmentSlotToInventory_Implementation(UFaerieEquipmentSlot* Slot,
																				UFaerieItemStorage* ToStorage)
{
	if (!IsValid(Slot)) return;
	if (!Slot->IsFilled()) return;
	if (!CanAccessSlot(Slot)) return;
	if (!IsValid(ToStorage)) return;
	if (!CanAccessStorage(ToStorage)) return;
	if (!ToStorage->CanAddStack({Slot->GetItem(), 1})) return;

	UFaerieItem* Item = Slot->TakeItemFromSlot();

	if (IsValid(Item))
	{
		ToStorage->AddEntryFromItemObject(Item);
	}
}