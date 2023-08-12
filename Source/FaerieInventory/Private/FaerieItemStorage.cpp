// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "FaerieItemStorage.h"

#include "FaerieItem.h"
#include "InventoryExtensionBase.h"
#include "LocalInventoryEntryCache.h"
#include "Net/UnrealNetwork.h"
#include "Tokens/FaerieItemStorageToken.h"
#include "Tokens/FaerieStackLimiterToken.h"

DECLARE_STATS_GROUP(TEXT("FaerieItemStorage"), STATGROUP_FaerieItemStorage, STATCAT_Advanced);
DECLARE_CYCLE_STAT(TEXT("Query (First)"), STAT_Storage_QueryFirst, STATGROUP_FaerieItemStorage);
DECLARE_CYCLE_STAT(TEXT("Query (All)"), STAT_Storage_QueryAll, STATGROUP_FaerieItemStorage);

DEFINE_LOG_CATEGORY(LogFaerieItemStorage);

void UFaerieItemStorage::PostInitProperties()
{
	Super::PostInitProperties();

	// Bind replication functions out into this class.
	EntryMap.ChangeListener = this;
}

void UFaerieItemStorage::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams SharedParams;
	SharedParams.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, EntryMap, SharedParams);
}

void UFaerieItemStorage::PostLoad()
{
	Super::PostLoad();

	// Determine the next valid key to use.
	NextKeyInt = !EntryMap.IsEmpty() ? EntryMap.GetKeyAt(EntryMap.Num()-1).Value()+1 : 100;
}

bool UFaerieItemStorage::IsValidKey(const FEntryKey Key) const
{
	return EntryMap.Contains(Key);
}

void UFaerieItemStorage::ForEachKey(const TFunctionRef<void(FEntryKey)>& Func) const
{
	TArray<FEntryKey> Keys;
	EntryMap.GetKeys(Keys);
	for (const FEntryKey Key : Keys)
	{
		Func(Key);
	}
}

FInventoryStack UFaerieItemStorage::GetStack(const FEntryKey Key) const
{
	if (!IsValidKey(Key)) return FInventoryStack::EmptyStack;

	FInventoryEntry Entry;
	GetEntry(Key, Entry);
	return Entry.StackSum();
}

FFaerieItemStack UFaerieItemStorage::Release(const FFaerieItemStackView Stack)
{
	const FEntryKey Key = FindItem(Stack.Item);
	FFaerieItemStack OutStack;
	if (TakeEntry(Key, OutStack, FFaerieItemStorageEvents::Get().Removal_Moving, Stack.Copies))
	{
		return OutStack;
	}
	return FFaerieItemStack();
}

bool UFaerieItemStorage::Possess(const FFaerieItemStack Stack)
{
	if (!IsValid(Stack.Item) ||
		Stack.Copies < 1) return false;

	return AddEntryFromStackImpl(Stack).Success;
}

void UFaerieItemStorage::PostContentAdded(const FKeyedInventoryEntry& Entry)
{
	if (!Entry.Key.IsValid())
	{
		UE_LOG(LogFaerieItemStorage, Warning, TEXT("Received Invalid PostContentAdded Key"))
		return;
	}

	OnKeyAddedCallback.Broadcast(this, Entry.Key);
	OnKeyAdded.Broadcast(this, Entry.Key);
}

void UFaerieItemStorage::PostContentChanged(const FKeyedInventoryEntry& Entry)
{
	if (!ensure(Entry.Key.IsValid()))
	{
		UE_LOG(LogFaerieItemStorage, Warning, TEXT("PostContentChanged: Received Invalid Key"))
		return;
	}

	if (!Entry.Entry.IsValid())
	{
		return;
	}

	if (IsValidKey(Entry.Key))
	{
		Extensions->PostEntryChanged(this, Entry.Key);

		OnKeyUpdatedCallback.Broadcast(this, Entry.Key);
		OnKeyUpdated.Broadcast(this, Entry.Key);

		// Call updates on any local views
		auto&& Keys = GetInvKeysForEntry(Entry.Key);
		for (auto&& Key : Keys)
		{
			if (auto&& Local = LocalCachedEntries.Find(Key))
			{
				if (Local->IsValid())
				{
					Local->Get()->NotifyUpdate();
				}
			}
		}
	}
	else
	{
		// Do nothing, PreContentRemoved should handle this . . .
	}
}

void UFaerieItemStorage::PreContentRemoved(const FKeyedInventoryEntry& Entry)
{
	if (!Entry.Key.IsValid())
	{
		UE_LOG(LogFaerieItemStorage, Warning, TEXT("Received Invalid PreContentRemoved Key"))
		return;
	}

	OnKeyRemovedCallback.Broadcast(this, Entry.Key);
	OnKeyRemoved.Broadcast(this, Entry.Key);

	// Cleanup local views.
	for (auto&& Stack : Entry.Entry.Stacks)
	{
		TWeakObjectPtr<UInventoryEntryProxy> Local;
		LocalCachedEntries.RemoveAndCopyValue({Entry.Key, Stack.Key}, Local);
		if (Local.IsValid())
		{
			Local->NotifyRemoval();
		}
	}
}


	/**------------------------------*/
	/*	  INTERNAL IMPLEMENTATIONS	 */
	/**------------------------------*/

const FInventoryEntry& UFaerieItemStorage::GetEntryImpl(const FEntryKey Key) const
{
	return EntryMap[Key];
}

bool UFaerieItemStorage::GetEntryImpl(const FEntryKey Key, FInventoryEntry& Entry) const
{
	check(IsValidKey(Key))
	Entry = EntryMap[Key];
	return true;
}

Faerie::FItemContainerEvent UFaerieItemStorage::AddEntryImpl(const FInventoryEntry& InEntry)
{
	struct FAdditionEventScope
	{
		FAdditionEventScope(UFaerieItemStorage* Storage)
		  : Storage(Storage) {}

		~FAdditionEventScope()
		{
			Storage->Extensions->PostAddition(Storage, Event);
		}

		Faerie::FItemContainerEvent Event;
		UFaerieItemStorage* Storage;
	} Scope(this);

	Extensions->PreAddition(this, {InEntry.ItemObject, InEntry.StackSum().GetAmount() });

	if (!ensureAlwaysMsgf(InEntry.IsValid(), TEXT("AddEntryImpl was passed an invalid entry.")))
	{
		return Faerie::FItemContainerEvent::AdditionFailed("AddEntryImpl was passed an invalid entry.");
	}

	// Log for this event
	Scope.Event.Type = FFaerieItemStorageEvents::Get().Addition;
	Scope.Event.Item = InEntry.ItemObject;
	Scope.Event.Amount = InEntry.StackSum().GetAmount();

	// Mutables cannot stack, due to, well, being mutable, meaning that each individual retains the ability to
	// uniquely mutate from others.
	if (!InEntry.ItemObject->IsDataMutable())
	{
		Scope.Event.EntryTouched = QueryFirst(FNativeStorageFilter::CreateLambda([InEntry](const FInventoryEntry& Other)
			{
				return FInventoryEntry::IsEqualTo(InEntry, Other, EEntryEquivelancyFlags::ItemData);
			})).Key;
	}

	TArray<FStackKey> StacksTouched;

	// Try to fill up the stacks of an existing entry first, before creating a new entry.
	if (Scope.Event.EntryTouched.IsValid())
	{
		const FInventoryContent::FScopedItemHandle& Entry = EntryMap.GetMutableHandle(Scope.Event.EntryTouched);
		Entry->AddToAnyStack(Scope.Event.Amount, &StacksTouched);
	}
	else
	{
		// NextKey() is guaranteed to have a greater value than all currently existing keys, so simply appending is fine, and
		// will keep the EntryMap sorted.
		Scope.Event.EntryTouched = NextKey();

		TakeOwnership(InEntry.ItemObject);

		/*FKeyedInventoryEntry& AddedEntry =*/ EntryMap.Append(Scope.Event.EntryTouched, InEntry);
		StacksTouched = InEntry.GetKeys();
	}

	Scope.Event.OtherKeysTouched.Append(StacksTouched);

	Scope.Event.Success = true;

	return Scope.Event;
}

Faerie::FItemContainerEvent UFaerieItemStorage::AddEntryFromStackImpl(const FFaerieItemStack InStack)
{
	FInventoryEntry Entry;
	Entry.ItemObject = InStack.Item;
	Entry.Limit = UFaerieStackLimiterToken::GetItemStackLimit(Entry.ItemObject);
	Entry.AddToAnyStack(InStack.Copies);

	return AddEntryImpl(Entry);
}

Faerie::FItemContainerEvent UFaerieItemStorage::AddEntryFromProxyImpl(UFaerieItemDataProxyBase* InProxy)
{
	check(InProxy);
	check(InProxy->GetOwner())

	const FFaerieItemStackView StackView = {InProxy->GetItemObject(), InProxy->GetCopies()};
	const FFaerieItemStack Stack = InProxy->GetOwner()->Release(StackView);

	return AddEntryFromStackImpl(Stack);
}

Faerie::FItemContainerEvent UFaerieItemStorage::RemoveFromEntryImpl(const FEntryKey Key, const FInventoryStack Amount,
                                                                const FFaerieInventoryTag Reason)
{
	// RemoveEntryImpl should not be called with unvalidated parameters.
	check(IsValidKey(Key));
	check(Amount != 0 && Amount >= FInventoryStack::UnlimitedStack);
	check(Reason.MatchesTag(FFaerieItemStorageEvents::Get().RemovalBaseTag))

	struct FRemovalEventScope
	{
		FRemovalEventScope(UFaerieItemStorage* Storage)
		  : Storage(Storage) {}

		~FRemovalEventScope()
		{
			Storage->Extensions->PostRemoval(Storage, Event);
		}

		Faerie::FItemContainerEvent Event;
		UFaerieItemStorage* Storage;
	} Scope(this);

	Extensions->PreRemoval(this, Key, Amount.GetAmount());

	// Log for this event
	Scope.Event.Type = Reason;
	Scope.Event.EntryTouched = Key;

	bool Remove = false;

	// Open Mutable Scope
	{
		const FInventoryContent::FScopedItemHandle& Handle = EntryMap.GetMutableHandle(Key);

		Scope.Event.Item = Handle->ItemObject;
		auto&& Sum = Handle->StackSum();

		TArray<FStackKey> StacksTouched;

		if (Amount == FInventoryStack::UnlimitedStack || Amount >= Sum) // Remove the entire entry
		{
			Scope.Event.Amount = Sum.GetAmount();
			StacksTouched = Handle->GetKeys();
			ReleaseOwnership(Handle->ItemObject);
			Remove = true;
		}
		else // Remove part of the entry
		{
			Scope.Event.Amount = FMath::Clamp(Amount.GetAmount(), 1, Sum.GetAmount()-1);
			Handle->RemoveFromAnyStack(Scope.Event.Amount, &StacksTouched);
		}

		Scope.Event.OtherKeysTouched.Append(StacksTouched);
	}
	// Close Mutable scope

	if (Remove)
	{
		UE_LOG(LogFaerieItemStorage, Log, TEXT("Removing entire entry at: '%s'"), *Key.ToString());
		EntryMap.Remove(Key);
	}

	return Scope.Event;
}

Faerie::FItemContainerEvent UFaerieItemStorage::RemoveFromStackImpl(const FInventoryKey Key, const FInventoryStack Amount,
																const FFaerieInventoryTag Reason)
{
	// RemoveEntryImpl should not be called with unvalidated parameters.
	check(Amount.IsValid());
	check(IsValidKey(Key.EntryKey));
	check(Reason.MatchesTag(FFaerieItemStorageEvents::Get().RemovalBaseTag))

	struct FRemovalEventScope
	{
		FRemovalEventScope(UFaerieItemStorage* Storage)
		  : Storage(Storage) {}

		~FRemovalEventScope()
		{
			Storage->Extensions->PostRemoval(Storage, Event);
		}

		Faerie::FItemContainerEvent Event;
		UFaerieItemStorage* Storage;
	} Scope(this);

	Extensions->PreRemoval(this, Key.EntryKey, Amount.GetAmount());

	// Log for this event
	Scope.Event.Type = Reason;
	Scope.Event.EntryTouched = Key.EntryKey;
	Scope.Event.OtherKeysTouched.Add(Key.StackKey);

	bool Remove = false;

	// Open Mutable Scope
	{
		auto&& Handle = EntryMap.GetMutableHandle(Key.EntryKey);

		Scope.Event.Item = Handle->ItemObject;

		auto&& Stack = Handle->GetStack(Key.StackKey);

		if (Amount == FInventoryStack::UnlimitedStack || Amount >= Stack) // Remove the entire stack
		{
			Scope.Event.Amount = Stack.GetAmount();

			Handle->Set(Key.StackKey, FInventoryStack::EmptyStack);

			if (Handle->Stacks.IsEmpty())
			{
				ReleaseOwnership(Handle->ItemObject);
				Remove = true;
			}
		}
		else // Remove part of the stack
		{
			check(Amount == FMath::Clamp(Amount.GetAmount(), 1, Stack.GetAmount()-1));

			Scope.Event.Amount = Amount.GetAmount();

			auto&& NewAmount = Stack - Amount;
			Handle->Set(Key.StackKey, NewAmount);
		}
	}
	// Close Mutable scope

	if (Remove)
	{
		UE_LOG(LogFaerieItemStorage, Log, TEXT("Removing entire stack at: '%s'"), *Key.ToString());
		EntryMap.Remove(Key.EntryKey);
	}

	// De-tally from total items.
	Scope.Event.Success = true;

	return Scope.Event;
}


	/**------------------------------*/
	/*	 STORAGE API - ALL USERS   */
	/**------------------------------*/

TArray<FInventoryKey> UFaerieItemStorage::GetInvKeysForEntry(const FEntryKey Key) const
{
	TArray<FInventoryKey> Out;

	if (!IsValidKey(Key)) return Out;

	auto&& Entry = GetEntryImpl(Key);
	for (auto&& Stack : Entry.Stacks)
	{
		Out.Add({Key, Stack.Key});
	}

	check(!Out.IsEmpty())

	return Out;
}

void UFaerieItemStorage::GetAllKeys(TArray<FEntryKey>& Keys) const
{
	EntryMap.GetKeys(Keys);
}

int32 UFaerieItemStorage::GetStackCount() const
{
	return EntryMap.Num();
}

bool UFaerieItemStorage::ContainsKey(const FEntryKey Key) const
{
	return IsValidKey(Key);
}

bool UFaerieItemStorage::IsValidKey(const FInventoryKey Key) const
{
	if (!IsValidKey(Key.EntryKey)) return false;
	auto&& Entry = GetEntryImpl(Key.EntryKey);
	return Entry.Stacks.FindByKey(Key.StackKey) != nullptr;
}

bool UFaerieItemStorage::ContainsItem(const UFaerieItem* Item) const
{
	return FindItem(Item).IsValid();
}

FEntryKey UFaerieItemStorage::FindItem(const UFaerieItem* Item) const
{
	return QueryFirst(FNativeStorageFilter::CreateLambda(
		[Item](const FInventoryEntry& Entry)
		{
			return Entry.ItemObject == Item;
		})).Key;
}

FInventoryKey UFaerieItemStorage::GetFirstKey() const
{
	if (EntryMap.IsEmpty()) return FInventoryKey();
	auto&& FirstEntry = EntryMap.Items[0];

	return { FirstEntry.Key, FirstEntry.Entry.Stacks[0].Key };
}

bool UFaerieItemStorage::GetEntry(const FEntryKey Key, FInventoryEntry& Entry) const
{
	if (!IsValidKey(Key)) return false;
	return GetEntryImpl(Key, Entry);
}

bool UFaerieItemStorage::GetProxyForEntry(const FInventoryKey Key, UInventoryEntryProxy*& Entry)
{
	if (!IsValidKey(Key.EntryKey))
	{
		Entry = nullptr;
		return false;
	}

	if (LocalCachedEntries.Contains(Key))
	{
		auto&& MaybeEntry = LocalCachedEntries[Key];
		if (MaybeEntry.IsValid())
		{
			Entry = MaybeEntry.Get();
			return true;
		}
	}

	const FName ProxyName = MakeUniqueObjectName(this, UInventoryEntryProxy::StaticClass(),
	                                             *FString::Printf(TEXT("ENTRY_PROXY_%i_%i"),
	                                             	Key.EntryKey.Value(), Key.StackKey.Value()));
	Entry = NewObject<UInventoryEntryProxy>(this, UInventoryEntryProxy::StaticClass(), ProxyName);
	check(IsValid(Entry));

	Entry->NotifyCreation(FInventoryKeyHandle{this, Key});

	LocalCachedEntries.Add(Key, Entry);

	return true;
}

void UFaerieItemStorage::GetEntryArray(const TArray<FEntryKey>& Keys, TArray<FInventoryEntry>& Entries) const
{
	// Allocate memory once
	Entries.SetNumUninitialized(Keys.Num(), true);
	for (int32 i = 0; i < Keys.Num(); ++i)
	{
		GetEntryImpl(Keys[i], Entries[i]);
	}
}

FKeyedInventoryEntry UFaerieItemStorage::QueryFirst(const FNativeStorageFilter& Filter) const
{
	SCOPE_CYCLE_COUNTER(STAT_Storage_QueryFirst);

	if (Filter.IsBound())
	{
		for (const FKeyedInventoryEntry& Item : EntryMap)
		{
			if (Filter.Execute(Item.Entry))
			{
				return Item;
			}
		}
	}

	return FKeyedInventoryEntry();
}

void UFaerieItemStorage::QueryAll(const FFaerieItemStorageNativeQuery& Query, TArray<FKeyedInventoryEntry>& OutKeys) const
{
	SCOPE_CYCLE_COUNTER(STAT_Storage_QueryAll);

	// Ensure we are starting with a blank slate.
	OutKeys.Empty();

	if (Query.Filter.IsBound())
	{
		if (Query.InvertFilter)
		{
			for (const FKeyedInventoryEntry& Item : EntryMap)
			{
				if (!Query.Filter.Execute(Item.Entry))
				{
					OutKeys.Add(Item);
				}
			}
		}
		else
		{
			for (const FKeyedInventoryEntry& Item : EntryMap)
			{
				if (Query.Filter.Execute(Item.Entry))
				{
					OutKeys.Add(Item);
				}
			}
		}
	}
	else
	{
		OutKeys = EntryMap.Items;
	}

	if (Query.Sort.IsBound())
	{
		if (Query.InvertSort)
		{
			Algo::Sort(OutKeys, [&, Sort = Query.Sort](const FKeyedInventoryEntry& A, const FKeyedInventoryEntry& B)
			{
				return !Sort.Execute(A.Entry, B.Entry);
			});
		}
		else
		{
			Algo::Sort(OutKeys, [&, Sort = Query.Sort](const FKeyedInventoryEntry& A, const FKeyedInventoryEntry& B)
			{
				return Sort.Execute(A.Entry, B.Entry);
			});
		}
	}
}

FEntryKey UFaerieItemStorage::QueryFirst(const FBlueprintStorageFilter& Filter) const
{
	if (!Filter.IsBound()) return FEntryKey();

	return QueryFirst(FNativeStorageFilter::CreateLambda(
		[Filter](const FInventoryEntry& Entry)
		{
			return Filter.Execute(Entry);
		})).Key;
}

void UFaerieItemStorage::QueryAll(const FFaerieItemStorageBlueprintQuery& Query,
	TArray<FEntryKey>& OutKeys) const
{
	FFaerieItemStorageNativeQuery NativeQuery;
	if (Query.Filter.IsBound())
	{
		NativeQuery.Filter.BindLambda(
			[Filter = Query.Filter](const FInventoryEntry& Entry)
			{
				return Filter.Execute(Entry);
			});
		NativeQuery.InvertFilter = Query.InvertFilter;
	}

	if (Query.Sort.IsBound())
	{
		NativeQuery.Sort.BindLambda(
			[Sort = Query.Sort](const FInventoryEntry& A ,const FInventoryEntry& B)
			{
				return Sort.Execute(A, B);
			});
		NativeQuery.InvertSort = Query.Reverse;
	}

	TArray<FKeyedInventoryEntry> Entries;
	QueryAll(NativeQuery, Entries);

	for (auto&& Entry : Entries)
	{
		OutKeys.Add(Entry.Key);
	}
}

bool UFaerieItemStorage::CanAddStack(const FFaerieItemStackView Stack) const
{
	if (!IsValid(Stack.Item) ||
		Stack.Copies < 1)
	{
		return false;
	}

	// Prevent recursive storage
	if (auto&& StorageToken = Stack.Item->GetToken<UFaerieItemStorageToken>())
	{
		if (StorageToken->GetItemStorage() == this)
		{
			return false;
		}
	}

	switch (Extensions->AllowsAddition(this, Stack)) {
	case EEventExtensionResponse::NoExplicitResponse:
	case EEventExtensionResponse::Allowed:				return true;
	case EEventExtensionResponse::Disallowed:			return false;
	default: return false;
	}
}

bool UFaerieItemStorage::CanAddProxy(UFaerieItemDataProxyBase* Proxy) const
{
	FFaerieItemStackView Stack;
	Stack.Item = Proxy->GetItemObject();
	Stack.Copies = Proxy->GetCopies();

	return CanAddStack(Stack);
}

bool UFaerieItemStorage::CanRemoveEntry(const FEntryKey Key, const FFaerieInventoryTag Reason) const
{
	// By default, some removal reasons are allowed, unless an extension explicitly disallows it.
	const bool DefaultAllowed = FFaerieItemStorageEvents::Get().RemovalTagsAllowedByDefault.Contains(Reason);

	switch (Extensions->AllowsRemoval(this, Key, Reason)) {
	case EEventExtensionResponse::NoExplicitResponse:	return DefaultAllowed;
	case EEventExtensionResponse::Allowed:				return true;
	case EEventExtensionResponse::Disallowed:			return false;
	default: return false;
	}
}

bool UFaerieItemStorage::CanRemoveStack(const FInventoryKey Key, const FFaerieInventoryTag Reason) const
{
	return CanRemoveEntry(Key.EntryKey, Reason);
}


	/**------------------------------*/
	/*	STORAGE API - SERVER ONLY  */
	/**------------------------------*/

bool UFaerieItemStorage::AddEntryFromItemObject(UFaerieItem* ItemObject)
{
	check(ItemObject);

	if (!CanAddStack({ItemObject, 1}))
	{
		return false;
	}

	FInventoryEntry Entry;
	Entry.ItemObject = ItemObject;
	Entry.Limit = UFaerieStackLimiterToken::GetItemStackLimit(ItemObject);
	Entry.AddToAnyStack(1);

	return AddEntryImpl(Entry).Success;
}

bool UFaerieItemStorage::AddItemStack(const FFaerieItemStack ItemStack)
{
	if (!CanAddStack(ItemStack))
	{
		return false;
	}

	return AddEntryFromStackImpl(ItemStack).Success;
}

bool UFaerieItemStorage::AddEntryFromProxy(UFaerieItemDataProxyBase* Proxy)
{
	if (!IsValid(Proxy))
	{
		UE_LOG(LogFaerieItemStorage, Warning, TEXT("AddEntryFromProxy called with invalid Proxy!"))
		return false;
	}

	if (!CanAddProxy(Proxy))
	{
		return false;
	}

	return AddEntryFromProxyImpl(Proxy).EntryTouched.IsValid();
}

void UFaerieItemStorage::AddEntriesFromProxy(const TArray<UFaerieItemDataProxyBase*>& Entries)
{
	for (auto&& Entry : Entries)
	{
		AddEntryFromProxy(Entry);
	}
}

bool UFaerieItemStorage::RemoveEntry(const FEntryKey Key, const FFaerieInventoryTag RemovalTag, const int32 Amount)
{
	if (Amount == 0 || Amount < -1) return false;
	if (!IsValidKey(Key)) return false;
	if (!CanRemoveEntry(Key, RemovalTag)) return false;

	const FInventoryStack Stack = Amount == FInventoryStack::UnlimitedNum ? FInventoryStack::UnlimitedStack : Amount;

	RemoveFromEntryImpl(Key, Stack, RemovalTag);
	return true;
}

bool UFaerieItemStorage::RemoveStack(const FInventoryKey Key, const FFaerieInventoryTag RemovalTag, const int32 Amount)
{
	if (Amount == 0 || Amount < -1) return false;
	if (!IsValidKey(Key.EntryKey)) return false;
	if (!CanRemoveStack(Key, RemovalTag)) return false;

	const FInventoryStack Stack = Amount == FInventoryStack::UnlimitedNum ? FInventoryStack::UnlimitedStack : Amount;

	RemoveFromStackImpl(Key, Stack, RemovalTag);
	return true;
}

bool UFaerieItemStorage::TakeEntry(const FEntryKey Key, FFaerieItemStack& OutStack,
                                          const FFaerieInventoryTag RemovalTag, const int32 Amount)
{
	if (Amount == 0 || Amount < -1) return false;
	if (!IsValidKey(Key)) return false;
	if (!CanRemoveEntry(Key, RemovalTag)) return false;

	FInventoryEntry Entry;
	if (!ensure(GetEntry(Key, Entry)))
	{
		return false;
	}

	const FInventoryStack Stack = Amount == FInventoryStack::UnlimitedNum ? FInventoryStack::UnlimitedStack : Amount;

	auto&& Event = RemoveFromEntryImpl(Key, Stack, RemovalTag);

	if (Event.Success)
	{
		OutStack.Item = Entry.ItemObject;
		OutStack.Copies = Event.Amount;
	}

	return Event.Success;
}

bool UFaerieItemStorage::TakeStack(const FInventoryKey Key, FFaerieItemStack& OutStack,
								   const FFaerieInventoryTag RemovalTag, const int32 Amount)
{
	if (Amount == 0 || Amount < -1) return false;
	if (!IsValidKey(Key.EntryKey)) return false;
	if (!CanRemoveStack(Key, RemovalTag)) return false;

	const FInventoryStack Stack = Amount == FInventoryStack::UnlimitedNum ? FInventoryStack::UnlimitedStack : Amount;

	FInventoryEntry Entry;
	if (!ensure(GetEntry(Key.EntryKey, Entry)))
	{
		return false;
	}

	auto&& Event = RemoveFromStackImpl(Key, Stack, RemovalTag);

	if (Event.Success)
	{
		OutStack.Item = Entry.ItemObject;
		OutStack.Copies = Event.Amount;
	}

	return Event.Success;
}

void UFaerieItemStorage::Clear()
{
	TArray<FEntryKey> KeysCopy;
	GetAllKeys(KeysCopy);
	for (FEntryKey Key : KeysCopy)
	{
		RemoveFromEntryImpl(Key, FInventoryStack::UnlimitedStack, FFaerieItemStorageEvents::Get().Removal_Deletion);
	}

	checkf(EntryMap.IsEmpty(), TEXT("Clear failed to empty EntryMap"));

#if WITH_EDITOR
	// The editor should reset next key, so that clearing and generating new content doesn't rack up the next key
	// endlessly.
	NextKeyInt = 100;
#endif
}

FEntryKey UFaerieItemStorage::MoveStack(UFaerieItemStorage* ToStorage, const FInventoryKey Key, const int32 Amount)
{
	if (!IsValid(ToStorage) ||
		!IsValidKey(Key.EntryKey) ||
		(Amount < FInventoryStack::UnlimitedNum || Amount == 0) ||
		!CanRemoveStack(Key, FFaerieItemStorageEvents::Get().Removal_Moving))
	{
		return FEntryKey::InvalidKey;
	}

	const FInventoryStack Stack = Amount == FInventoryStack::UnlimitedNum ? FInventoryStack::UnlimitedStack : Amount;

	FFaerieItemStack ItemStack;

	FInventoryEntry Entry;
	GetEntry(Key.EntryKey, Entry);
	ItemStack.Item = Entry.ItemObject;
	ItemStack.Copies = Stack == FInventoryStack::UnlimitedStack ? Entry.StackSum().GetAmount() : FMath::Min(Stack.GetAmount(), Entry.StackSum().GetAmount());

	if (!ToStorage->CanAddStack(ItemStack))
	{
		return FEntryKey::InvalidKey;
	}

	const Faerie::FItemContainerEvent Removed = RemoveFromStackImpl(Key, Stack, FFaerieItemStorageEvents::Get().Removal_Moving);

	if (!ensure(Removed.Success))
	{
		return FEntryKey::InvalidKey;
	}

	ItemStack.Copies = Removed.Amount;

	return ToStorage->AddEntryFromStackImpl(ItemStack).EntryTouched;
}

FEntryKey UFaerieItemStorage::MoveEntry(UFaerieItemStorage* ToStorage, const FEntryKey Key)
{
	if (!IsValid(ToStorage) ||
		!IsValidKey(Key) ||
		!CanRemoveEntry(Key, FFaerieItemStorageEvents::Get().Removal_Moving))
	{
		return FEntryKey::InvalidKey;
	}

	FInventoryEntry Entry;
	GetEntry(Key, Entry);
	if (!ToStorage->CanAddStack(Entry.ToItemStack()))
	{
		return FEntryKey::InvalidKey;
	}

	auto&& Result = RemoveFromEntryImpl(Key, FInventoryStack::UnlimitedStack, FFaerieItemStorageEvents::Get().Removal_Moving);

	if (!ensure(Result.Success))
	{
		return FEntryKey::InvalidKey;
	}

	return ToStorage->AddEntryFromStackImpl({Entry.ItemObject, Result.Amount}).EntryTouched;
}

void UFaerieItemStorage::Dump(UFaerieItemStorage* ToStorage)
{
	TArray<FEntryKey> KeysCopy;
	GetAllKeys(KeysCopy);
	for (FEntryKey Key : KeysCopy)
	{
		MoveEntry(ToStorage, Key);
	}
}