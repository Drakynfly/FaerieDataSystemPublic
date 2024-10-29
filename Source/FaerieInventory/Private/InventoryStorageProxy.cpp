// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "InventoryStorageProxy.h"
#include "FaerieItem.h"
#include "FaerieItemStorage.h"
#include "Logging.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(InventoryStorageProxy)

TArray<FKeyedStack> UInventoryEntryProxyBase::GetAllStacks() const
{
	if (auto&& Entry = GetInventoryEntry();
		ensure(Entry.IsValid()))
	{
		return Entry.Stacks;
	}
	return {};
}

int32 UInventoryEntryProxyBase::GetStackLimit() const
{
	if (auto&& Entry = GetInventoryEntry();
		ensure(Entry.IsValid()))
	{
		return Entry.Limit;
	}
	return 0;
}

const UFaerieItem* UInventoryEntryStorageProxy::GetItemObject() const
{
	if (!VerifyStatus())
	{
		return nullptr;
	}

	const FConstStructView EntryView = GetStorage()->GetEntryView(GetKey());
	if (!ensure(EntryView.IsValid()))
	{
		return nullptr;
	}

	return EntryView.Get<const FInventoryEntry>().ItemObject;
}

int32 UInventoryEntryStorageProxy::GetCopies() const
{
	if (!VerifyStatus())
	{
		return 0;
	}

	const FConstStructView EntryView = GetStorage()->GetEntryView(GetKey());
	if (!ensure(EntryView.IsValid()))
	{
		return 0;
	}

	return EntryView.Get<const FInventoryEntry>().StackSum();
}

bool UInventoryEntryStorageProxy::CanMutate() const
{
	if (!VerifyStatus())
	{
		return false;
	}

	return GetStorage()->CanEditEntry(GetKey());
}

TScriptInterface<IFaerieItemOwnerInterface> UInventoryEntryStorageProxy::GetOwner() const
{
	return GetStorage();
}

FInventoryEntry UInventoryEntryStorageProxy::GetInventoryEntry() const
{
	if (!VerifyStatus())
	{
		return FInventoryEntry();
	}

	FInventoryEntry Entry;
	GetStorage()->GetEntry(GetKey(), Entry);
	return Entry;
}

void UInventoryEntryStorageProxy::NotifyCreation()
{
	LocalItemVersion = 0;
}

void UInventoryEntryStorageProxy::NotifyUpdate()
{
	LocalItemVersion++;
	OnCacheUpdatedNative.Broadcast(this);
	OnCacheUpdated.Broadcast(this);
}

void UInventoryEntryStorageProxy::NotifyRemoval()
{
	LocalItemVersion = -1;
	OnCacheRemovedNative.Broadcast(this);
	OnCacheRemoved.Broadcast(this);
}

bool UInventoryEntryStorageProxy::VerifyStatus() const
{
	auto&& Storage = GetStorage();
	auto&& Key = GetKey();

	if (!IsValid(Storage) || !Storage->IsValidKey(Key))
	{
		UE_LOG(LogFaerieInventory, Warning, TEXT("InventoryEntryProxy is invalid! Debug State will follow:"))\
		UE_LOG(LogFaerieInventory, Warning, TEXT("     Entry Cache: %s"), *GetName());
		UE_LOG(LogFaerieInventory, Warning, TEXT("     OwningInventory: %s"), IsValid(Storage) ? *Storage->GetName() : TEXT("Invalid"));
		UE_LOG(LogFaerieInventory, Warning, TEXT("     Key: %s"), *Key.ToString());
		UE_LOG(LogFaerieInventory, Warning, TEXT("     Item Version : %i"), LocalItemVersion);
		return false;
	}

	return true;
}

FEntryKey UInventoryEntryProxy::GetKey() const
{
	return Key;
}

int32 UInventoryStackProxy::GetCopies() const
{
	if (!VerifyStatus())
	{
		return 0;
	}

	const FConstStructView EntryView = ItemStorage->GetEntryView(Key.EntryKey);
	if (!ensure(EntryView.IsValid()))
	{
		return 0;
	}

	return EntryView.Get<const FInventoryEntry>().GetStack(Key.StackKey);
}

FEntryKey UInventoryStackProxy::GetKey() const
{
	return Key.EntryKey;
}

FInventoryKeyHandle UInventoryStackProxy::GetHandle() const
{
	return { ItemStorage, Key };
}