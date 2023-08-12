// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "LocalInventoryEntryCache.h"
#include "FaerieItem.h"
#include "FaerieItemStorage.h"
#include "Logging.h"
#include "Tokens/FaerieStackLimiterToken.h"

TArray<FKeyedStack> UInventoryEntryProxyBase::GetAllStacks() const
{
	auto&& Entry = GetInventoryEntry();
	if (ensure(Entry.IsValid()))
	{
		return Entry.Stacks;
	}
	return {};
}

FInventoryStack UInventoryEntryProxyBase::GetStackLimit() const
{
	auto&& Entry = GetInventoryEntry();
	if (ensure(Entry.IsValid()))
	{
		return Entry.Limit;
	}
	return FInventoryStack();
}

FDateTime UInventoryEntryProxyBase::GetDateModified() const
{
	auto&& Entry = GetInventoryEntry();
	if (ensure(Entry.IsValid()))
	{
		return Entry.ItemObject->GetLastModified();
	}
	return FDateTime();
}

const UFaerieItem* UInventoryEntryLiteral::GetItemObject() const
{
	return Entry.ItemObject;
}

int32 UInventoryEntryLiteral::GetCopies() const
{
	return Entry.StackSum().GetAmount();
}

FFaerieItemStack UInventoryEntryLiteral::Release(const FFaerieItemStackView Stack)
{
	FFaerieItemStack OutStack;

	if (Entry.ItemObject == Stack.Item &&
		Entry.StackSum().GetAmount() >= Stack.Copies)
	{
		Entry.RemoveFromAnyStack(Stack.Copies);
		OutStack.Item = Entry.ItemObject;
		OutStack.Copies = Stack.Copies;

		if (Entry.StackSum().GetAmount() <= 0)
		{
			Entry = FInventoryEntry();
		}
	}

	return OutStack;
}

bool UInventoryEntryLiteral::Possess(const FFaerieItemStack Stack)
{
	// @todo this is wonky behavior
	if (Entry.ItemObject == Stack.Item)
	{
		Entry.AddToAnyStack(Stack.Copies);
	}
	else
	{
		Entry.ItemObject = Stack.Item;
		Entry.Limit = UFaerieStackLimiterToken::GetItemStackLimit(Stack.Item);
		Entry.AddToAnyStack(Stack.Copies);
	}
	return true;
}

FInventoryEntry UInventoryEntryLiteral::GetInventoryEntry() const
{
	return Entry;
}

void UInventoryEntryLiteral::SetValue(const FInventoryEntry& InEntry)
{
	Entry = InEntry;
}

UInventoryEntryLiteral* UInventoryEntryLiteral::CreateInventoryEntryLiteral(const FInventoryEntry& Entry)
{
	auto&& Literal = NewObject<UInventoryEntryLiteral>();
	Literal->SetValue(Entry);
	return Literal;
}

const UFaerieItem* UInventoryEntryProxy::GetItemObject() const
{
	if (!VerifyStatus())
	{
		return nullptr;
	}

	FInventoryEntry Entry;
	Handle.ItemStorage->GetEntry(Handle.Key.EntryKey, Entry);
	return Entry.ItemObject;
}

int32 UInventoryEntryProxy::GetCopies() const
{
	if (!VerifyStatus())
	{
		return 0;
	}

	FInventoryEntry Entry;
	Handle.ItemStorage->GetEntry(Handle.Key.EntryKey, Entry);
	return Entry.GetStack(Handle.Key.StackKey).GetAmount();
}

TScriptInterface<IFaerieItemOwnerInterface> UInventoryEntryProxy::GetOwner()
{
	return Handle.ItemStorage.Get();
}

FInventoryEntry UInventoryEntryProxy::GetInventoryEntry() const
{
	if (!VerifyStatus())
	{
		return FInventoryEntry();
	}

	FInventoryEntry Entry;
	Handle.ItemStorage->GetEntry(Handle.Key.EntryKey, Entry);
	return Entry;
}

FInventoryStack UInventoryEntryProxy::GetStack() const
{
	auto&& Entry = GetInventoryEntry();
	return Entry.GetStack(Handle.Key.StackKey);
}

void UInventoryEntryProxy::NotifyCreation(const FInventoryKeyHandle& InHandle)
{
	Handle = InHandle;
	LocalItemVersion = 0;
}

void UInventoryEntryProxy::NotifyUpdate()
{
	LocalItemVersion++;
	OnCacheUpdated.Broadcast(this);
}

void UInventoryEntryProxy::NotifyRemoval()
{
	LocalItemVersion = -1;
	OnCacheRemoved.Broadcast(this);
}

bool UInventoryEntryProxy::VerifyStatus() const
{
	if (!Handle.ItemStorage.IsValid() || !Handle.ItemStorage.Get()->IsValidKey(Handle.Key))
	{
		UE_LOG(LogFaerieInventory, Warning, TEXT("LocalInventoryEntryCache is invalid! Debug State will follow:"))\
		UE_LOG(LogFaerieInventory, Warning, TEXT("     Entry Cache: %s"), *GetName());
		UE_LOG(LogFaerieInventory, Warning, TEXT("     OwningInventory: %s"), Handle.ItemStorage.IsValid() ? *Handle.ItemStorage.Get()->GetName() : TEXT("Invalid"));
		UE_LOG(LogFaerieInventory, Warning, TEXT("     Key: %s"), *Handle.Key.ToString());
		UE_LOG(LogFaerieInventory, Warning, TEXT("     Item Version : %i"), LocalItemVersion);
		return false;
	}

	return true;
}
