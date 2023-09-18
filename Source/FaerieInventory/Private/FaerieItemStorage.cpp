// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "FaerieItemStorage.h"

#include "FaerieItem.h"
#include "FaerieItemDataLibrary.h"
#include "ItemContainerExtensionBase.h"
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
	// See Footnote1
}

bool UFaerieItemStorage::IsValidKey(const FEntryKey Key) const
{
	return EntryMap.Contains(Key);
}

FFaerieItemStackView UFaerieItemStorage::View(const FEntryKey Key) const
{
	return EntryMap[Key].ToItemStackView();
}

FFaerieItemProxy UFaerieItemStorage::Proxy(const FEntryKey Key) const
{
	return GetEntryProxyImpl(Key);
}

void UFaerieItemStorage::ForEachKey(const TFunctionRef<void(FEntryKey)>& Func) const
{
	for (const FKeyedInventoryEntry& Element : EntryMap)
	{
		Func(Element.Key);
	}
}

int32 UFaerieItemStorage::GetStack(const FEntryKey Key) const
{
	if (!IsValidKey(Key)) return 0;

	// Return the total items stored by this key, across all stacks, since this API doesn't know about stacks.
	return GetEntryViewImpl(Key).Get<const FInventoryEntry>().StackSum();
}

void UFaerieItemStorage::OnItemMutated(const UFaerieItem* Item, const UFaerieItemToken* Token)
{
	Super::OnItemMutated(Item, Token);

	// @todo annoying but acceptable
	for (const FKeyedInventoryEntry& Element : EntryMap)
	{
		if (Element.Value.ItemObject == Item)
		{
			PostContentChanged(Element);
			return;
		}
	}
}

FFaerieItemStack UFaerieItemStorage::Release(const FFaerieItemStackView Stack)
{
	const FEntryKey Key = FindItem(Stack.Item);
	if (FFaerieItemStack OutStack;
		TakeEntry(Key, OutStack, FFaerieItemStorageEvents::Get().Removal_Moving, Stack.Copies))
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

	if (!Entry.Value.IsValid())
	{
		return;
	}

	if (IsValidKey(Entry.Key))
	{
		Extensions->PostEntryChanged(this, Entry.Key);

		OnKeyUpdatedCallback.Broadcast(this, Entry.Key);
		OnKeyUpdated.Broadcast(this, Entry.Key);

		// Call updates on any local views
		for (auto&& Keys = GetInvKeysForEntry(Entry.Key);
			auto&& Key : Keys)
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
	for (auto&& Stack : Entry.Value.Stacks)
	{
		TWeakObjectPtr<UInventoryStackProxy> Local;
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

FConstStructView UFaerieItemStorage::GetEntryViewImpl(const FEntryKey Key) const
{
	return FConstStructView::Make(EntryMap[Key]);
}

UInventoryEntryProxy* UFaerieItemStorage::GetEntryProxyImpl(const FEntryKey Key) const
{
	if (!IsValidKey(Key))
	{
		return nullptr;
	}

	if (EntryProxies.Contains(Key))
	{
		if (auto&& MaybeEntry = EntryProxies[Key];
			MaybeEntry.IsValid())
		{
			return MaybeEntry.Get();
		}
	}

	ThisClass* This = const_cast<ThisClass*>(this);

	const FName ProxyName = MakeUniqueObjectName(This, UInventoryEntryProxy::StaticClass(),
												 *FString::Printf(TEXT("ENTRY_PROXY_%s"), *Key.ToString()));
	auto&& Entry = NewObject<UInventoryEntryProxy>(This, UInventoryEntryProxy::StaticClass(), ProxyName);
	check(IsValid(Entry));

	Entry->Key = Key;
	Entry->ItemStorage = This;

	This->EntryProxies.Add(Key, Entry);

	return Entry;
}

void UFaerieItemStorage::GetEntryImpl(const FEntryKey Key, FInventoryEntry& Entry) const
{
	check(IsValidKey(Key))
	Entry = EntryMap[Key];
}

Faerie::Inventory::FEventLog UFaerieItemStorage::AddEntryImpl(const FInventoryEntry& InEntry)
{
	if (!ensureAlwaysMsgf(InEntry.IsValid(), TEXT("AddEntryImpl was passed an invalid entry.")))
	{
		return Faerie::Inventory::FEventLog::AdditionFailed("AddEntryImpl was passed an invalid entry.");
	}

	Faerie::Inventory::FEventLog Event;

	// Setup Log for this event
	Event.Type = FFaerieItemStorageEvents::Get().Addition;
	Event.Item = InEntry.ItemObject;
	Event.Amount = InEntry.StackSum();

	// Mutables cannot stack, due to, well, being mutable, meaning that each individual retains the ability to
	// uniquely mutate from others.
	if (!InEntry.ItemObject->IsDataMutable())
	{
		Event.EntryTouched = QueryFirst(FNativeStorageFilter::CreateLambda([InEntry](const FFaerieItemProxy& Other)
			{
				return UFaerieItemDataLibrary::Equal_ItemData(InEntry.ItemObject, Other.GetItemObject());
			})).Key;
	}

	// Execute PreAddition on all extensions
	Extensions->PreAddition(this, {InEntry.ItemObject, Event.Amount });

	TArray<FStackKey> StacksTouched;

	// Try to fill up the stacks of an existing entry first, before creating a new entry.
	if (Event.EntryTouched.IsValid())
	{
		const FInventoryContent::FScopedItemHandle& Entry = EntryMap.GetHandle(Event.EntryTouched);
		Entry->AddToAnyStack(Event.Amount, &StacksTouched);
	}
	else
	{
		// NextKey() is guaranteed to have a greater value than all currently existing keys, so simply appending is fine, and
		// will keep the EntryMap sorted.
		Event.EntryTouched = NextKey();

		TakeOwnership(InEntry.ItemObject);

		/*FKeyedInventoryEntry& AddedEntry =*/ EntryMap.Append(Event.EntryTouched, InEntry);
		StacksTouched = InEntry.CopyKeys();
	}

	Event.OtherKeysTouched.Append(StacksTouched);

	Event.Success = true;

	// Execute PostAddition on all extensions with the finished Event
	Extensions->PostAddition(this, Event);

	return Event;
}

Faerie::Inventory::FEventLog UFaerieItemStorage::AddEntryFromStackImpl(const FFaerieItemStack InStack)
{
	FInventoryEntry Entry;
	Entry.ItemObject = InStack.Item;
	Entry.Limit = UFaerieStackLimiterToken::GetItemStackLimit(Entry.ItemObject);
	Entry.AddToAnyStack(InStack.Copies);

	return AddEntryImpl(Entry);
}

Faerie::Inventory::FEventLog UFaerieItemStorage::RemoveFromEntryImpl(const FEntryKey Key, const int32 Amount,
                                                                const FFaerieInventoryTag Reason)
{
	// RemoveEntryImpl should not be called with unvalidated parameters.
	check(IsValidKey(Key));
	check(Faerie::ItemData::IsValidStack(Amount));
	check(Reason.MatchesTag(FFaerieItemStorageEvents::Get().RemovalBaseTag))

	Faerie::Inventory::FEventLog Event;

	Extensions->PreRemoval(this, Key, Amount);

	// Log for this event
	Event.Type = Reason;
	Event.EntryTouched = Key;

	bool Remove = false;

	// Open Mutable Scope
	{
		const FInventoryContent::FScopedItemHandle& Handle = EntryMap.GetHandle(Key);

		Event.Item = Handle->ItemObject;
		auto&& Sum = Handle->StackSum();

		TArray<FStackKey> StacksTouched;

		if (Amount == Faerie::ItemData::UnlimitedStack || Amount >= Sum) // Remove the entire entry
		{
			Event.Amount = Sum;
			StacksTouched = Handle->CopyKeys();
			ReleaseOwnership(Handle->ItemObject);
			Remove = true;
		}
		else // Remove part of the entry
		{
			Event.Amount = FMath::Clamp(Amount, 1, Sum-1);
			Handle->RemoveFromAnyStack(Event.Amount, &StacksTouched);
		}

		Event.OtherKeysTouched.Append(StacksTouched);
	}
	// Close Mutable scope

	if (Remove)
	{
		UE_LOG(LogFaerieItemStorage, Log, TEXT("Removing entire entry at: '%s'"), *Key.ToString());
		EntryMap.Remove(Key);
	}

	Extensions->PostRemoval(this, Event);

	return Event;
}

Faerie::Inventory::FEventLog UFaerieItemStorage::RemoveFromStackImpl(const FInventoryKey Key, const int32 Amount,
																const FFaerieInventoryTag Reason)
{
	// RemoveEntryImpl should not be called with unvalidated parameters.
	check(Faerie::ItemData::IsValidStack(Amount));
	check(IsValidKey(Key.EntryKey));
	check(Reason.MatchesTag(FFaerieItemStorageEvents::Get().RemovalBaseTag))

	Faerie::Inventory::FEventLog Event;

	Extensions->PreRemoval(this, Key.EntryKey, Amount);

	// Log for this event
	Event.Type = Reason;
	Event.EntryTouched = Key.EntryKey;
	Event.OtherKeysTouched.Add(Key.StackKey);

	bool Remove = false;

	// Open Mutable Scope
	{
		auto&& Handle = EntryMap.GetHandle(Key.EntryKey);

		Event.Item = Handle->ItemObject;

		if (auto&& Stack = Handle->GetStack(Key.StackKey);
			Amount == Faerie::ItemData::UnlimitedStack || Amount >= Stack) // Remove the entire stack
		{
			Event.Amount = Stack;

			Handle->SetStack(Key.StackKey, 0);

			if (Handle->Stacks.IsEmpty())
			{
				ReleaseOwnership(Handle->ItemObject);
				Remove = true;
			}
		}
		else // Remove part of the stack
		{
			check(Amount == FMath::Clamp(Amount, 1, Stack-1));

			Event.Amount = Amount;

			auto&& NewAmount = Stack - Amount;
			Handle->SetStack(Key.StackKey, NewAmount);
		}
	}
	// Close Mutable scope

	if (Remove)
	{
		UE_LOG(LogFaerieItemStorage, Log, TEXT("Removing entire stack at: '%s'"), *Key.ToString());
		EntryMap.Remove(Key.EntryKey);
	}

	// De-tally from total items.
	Event.Success = true;

	Extensions->PostRemoval(this, Event);

	return Event;
}


	/**------------------------------*/
	/*	 STORAGE API - ALL USERS   */
	/**------------------------------*/

FConstStructView UFaerieItemStorage::GetEntryView(const FEntryKey Key) const
{
	return GetEntryViewImpl(Key);
}

TArray<FInventoryKey> UFaerieItemStorage::GetInvKeysForEntry(const FEntryKey Key) const
{
	TArray<FInventoryKey> Out;

	if (!IsValidKey(Key)) return Out;

	for (const FConstStructView Entry = GetEntryViewImpl(Key);
		const FKeyedStack& Stack : Entry.Get<const FInventoryEntry>().Stacks)
	{
		Out.Add({Key, Stack.Key});
	}

	check(!Out.IsEmpty())

	return Out;
}

void UFaerieItemStorage::GetAllKeys(TArray<FEntryKey>& Keys) const
{
	Keys.Empty(EntryMap.Num());
	Algo::Transform(EntryMap, Keys, &FKeyedInventoryEntry::Key);
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
	return GetEntryViewImpl(Key.EntryKey).Get<const FInventoryEntry>().Stacks.FindByKey(Key.StackKey) != nullptr;
}

bool UFaerieItemStorage::ContainsItem(const UFaerieItem* Item) const
{
	return FindItem(Item).IsValid();
}

FEntryKey UFaerieItemStorage::FindItem(const UFaerieItem* Item) const
{
	return QueryFirst(FNativeStorageFilter::CreateLambda(
		[Item](const FFaerieItemProxy& Proxy)
		{
			return Proxy.GetItemObject() == Item;
		})).Key;
}

FInventoryKey UFaerieItemStorage::GetFirstKey() const
{
	if (EntryMap.IsEmpty()) return FInventoryKey();
	auto&& FirstEntry = EntryMap.Entries[0];

	return { FirstEntry.Key, FirstEntry.Value.Stacks[0].Key };
}

bool UFaerieItemStorage::GetEntry(const FEntryKey Key, FInventoryEntry& Entry) const
{
	if (!IsValidKey(Key)) return false;
	GetEntryImpl(Key, Entry);
	return true;
}

bool UFaerieItemStorage::GetProxyForEntry(const FInventoryKey Key, UInventoryStackProxy*& Entry)
{
	if (!IsValidKey(Key.EntryKey))
	{
		Entry = nullptr;
		return false;
	}

	if (LocalCachedEntries.Contains(Key))
	{
		if (auto&& MaybeEntry = LocalCachedEntries[Key];
			MaybeEntry.IsValid())
		{
			Entry = MaybeEntry.Get();
			return true;
		}
	}

	const FName ProxyName = MakeUniqueObjectName(this, UInventoryStackProxy::StaticClass(),
	                                             *FString::Printf(TEXT("ENTRY_PROXY_%s_%s"),
	                                             *Key.EntryKey.ToString(), *Key.StackKey.ToString()));
	Entry = NewObject<UInventoryStackProxy>(this, UInventoryStackProxy::StaticClass(), ProxyName);
	check(IsValid(Entry));

	Entry->Handle = FInventoryKeyHandle{this, Key};
	Entry->NotifyCreation();

	LocalCachedEntries.Add(Key, Entry);

	return true;
}

bool UFaerieItemStorage::GetStackProxy(const FInventoryKey Key, FFaerieItemProxy& Proxy)
{
	UInventoryStackProxy* StackProxy = nullptr;
	GetProxyForEntry(Key, StackProxy);
	Proxy = {StackProxy};
	return Proxy.IsValid();
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
			if (Filter.Execute(Proxy(Item.Key)))
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
			Algo::CopyIf(EntryMap, OutKeys,
				[&Query, this](const FKeyedInventoryEntry& Item)
				{
					return !Query.Filter.Execute(Proxy(Item.Key));
				});
		}
		else
		{
			Algo::CopyIf(EntryMap, OutKeys,
				[&Query, this](const FKeyedInventoryEntry& Item)
				{
					return Query.Filter.Execute(Proxy(Item.Key));
				});
		}
	}
	else
	{
		OutKeys = EntryMap.Entries;
	}

	if (Query.Sort.IsBound())
	{
		if (Query.InvertSort)
		{
			Algo::Sort(OutKeys,
				[&, Sort = Query.Sort](const FKeyedInventoryEntry& A, const FKeyedInventoryEntry& B)
				{
					return !Sort.Execute(Proxy(A.Key), Proxy(B.Key));
				});
		}
		else
		{
			Algo::Sort(OutKeys,
				[&, Sort = Query.Sort](const FKeyedInventoryEntry& A, const FKeyedInventoryEntry& B)
				{
					return Sort.Execute(Proxy(A.Key), Proxy(B.Key));
				});
		}
	}
}

FEntryKey UFaerieItemStorage::QueryFirst(const FBlueprintStorageFilter& Filter) const
{
	if (!Filter.IsBound()) return FEntryKey();

	return QueryFirst(FNativeStorageFilter::CreateLambda(
		[Filter](const FFaerieItemProxy& Proxy)
		{
			return Filter.Execute(Proxy);
		})).Key;
}

void UFaerieItemStorage::QueryAll(const FFaerieItemStorageBlueprintQuery& Query, TArray<FEntryKey>& OutKeys) const
{
	FFaerieItemStorageNativeQuery NativeQuery;
	if (Query.Filter.IsBound())
	{
		NativeQuery.Filter.BindLambda(
			[Filter = Query.Filter](const FFaerieItemProxy& Proxy)
			{
				return Filter.Execute(Proxy);
			});
		NativeQuery.InvertFilter = Query.InvertFilter;
	}

	if (Query.Sort.IsBound())
	{
		NativeQuery.Sort.BindLambda(
			[Sort = Query.Sort](const FFaerieItemProxy& A, const FFaerieItemProxy& B)
			{
				return Sort.Execute(A, B);
			});
		NativeQuery.InvertSort = Query.Reverse;
	}

	TArray<FKeyedInventoryEntry> Entries;
	QueryAll(NativeQuery, Entries);
	OutKeys.Reserve(Entries.Num());
	Algo::Transform(Entries, OutKeys, &FKeyedInventoryEntry::Key);
}

bool UFaerieItemStorage::CanAddStack(const FFaerieItemStackView Stack) const
{
	if (!IsValid(Stack.Item) ||
		Stack.Copies < 1)
	{
		return false;
	}

	// Prevent recursive storage
	// @todo this only checks one layer of depth. Theoretically, this storage token could point to some other ItemStorage,
	// which in turn has an item that points to us, which will crash the Extensions code when the item is possessed,
	// but honestly, I don't feel like fixing that unless it becomes an problem.
	if (auto&& StorageToken = Stack.Item->GetToken<UFaerieItemContainerToken>())
	{
		if (StorageToken->GetItemContainer() == this)
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

bool UFaerieItemStorage::CanEditEntry(FEntryKey EntryKey)
{
	// @todo Expose this to extensions
	return true;
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


	/**---------------------------------*/
	/*	 STORAGE API - AUTHORITY ONLY   */
	/**---------------------------------*/

bool UFaerieItemStorage::AddEntryFromItemObject(UFaerieItem* ItemObject)
{
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

bool UFaerieItemStorage::RemoveEntry(const FEntryKey Key, const FGameplayTag RemovalTag, const int32 Amount)
{
	if (Amount == 0 || Amount < -1) return false;
	if (!IsValidKey(Key)) return false;
	if (!CanRemoveEntry(Key, FFaerieInventoryTag::TryConvert(RemovalTag))) return false;

	const int32 Stack = Amount == Faerie::ItemData::UnlimitedStack ? Faerie::ItemData::UnlimitedStack : Amount;

	RemoveFromEntryImpl(Key, Stack, FFaerieInventoryTag::TryConvert(RemovalTag));
	return true;
}

bool UFaerieItemStorage::RemoveStack(const FInventoryKey Key, const FGameplayTag RemovalTag, const int32 Amount)
{
	if (Amount == 0 || Amount < -1) return false;
	if (!IsValidKey(Key.EntryKey)) return false;

	const FFaerieInventoryTag Tag = FFaerieInventoryTag::TryConvert(RemovalTag);
	if (!Tag.IsValid()) return false;

	if (!CanRemoveStack(Key, Tag)) return false;

	const int32 Stack = Amount == Faerie::ItemData::UnlimitedStack ? Faerie::ItemData::UnlimitedStack : Amount;

	RemoveFromStackImpl(Key, Stack, Tag);
	return true;
}

bool UFaerieItemStorage::TakeEntry(const FEntryKey Key, FFaerieItemStack& OutStack,
                                   const FGameplayTag RemovalTag, const int32 Amount)
{
	if (Amount == 0 || Amount < -1) return false;
	if (!IsValidKey(Key)) return false;

	const FFaerieInventoryTag Tag = FFaerieInventoryTag::TryConvert(RemovalTag);
	if (!Tag.IsValid()) return false;

	if (!CanRemoveEntry(Key, Tag)) return false;

	const int32 Stack = Amount == Faerie::ItemData::UnlimitedStack ? Faerie::ItemData::UnlimitedStack : Amount;

	auto&& Event = RemoveFromEntryImpl(Key, Stack, Tag);

	if (Event.Success)
	{
		OutStack.Item = const_cast<UFaerieItem*>(Event.Item.Get());
		OutStack.Copies = Event.Amount;
	}

	return Event.Success;
}

bool UFaerieItemStorage::TakeStack(const FInventoryKey Key, FFaerieItemStack& OutStack,
								   const FGameplayTag RemovalTag, const int32 Amount)
{
	if (Amount == 0 || Amount < -1) return false;
	if (!IsValidKey(Key.EntryKey)) return false;

	const FFaerieInventoryTag Tag = FFaerieInventoryTag::TryConvert(RemovalTag);
	if (!Tag.IsValid()) return false;

	if (!CanRemoveStack(Key, Tag)) return false;

	const int32 Stack = Amount == Faerie::ItemData::UnlimitedStack ? Faerie::ItemData::UnlimitedStack : Amount;

	auto&& Event = RemoveFromStackImpl(Key, Stack, Tag);

	if (Event.Success)
	{
		OutStack.Item = const_cast<UFaerieItem*>(Event.Item.Get());
		OutStack.Copies = Event.Amount;
	}

	return Event.Success;
}

void UFaerieItemStorage::Clear(const FGameplayTag RemovalTag)
{
	FFaerieInventoryTag Tag = FFaerieInventoryTag::TryConvert(RemovalTag);

	if (!Tag.IsValid())
	{
		Tag = FFaerieItemStorageEvents::Get().Removal_Deletion;
	}

	for (const FKeyedInventoryEntry& Element : EntryMap)
	{
		RemoveFromEntryImpl(Element.Key, Faerie::ItemData::UnlimitedStack, Tag);
	}

	checkf(EntryMap.IsEmpty(), TEXT("Clear failed to empty EntryMap"));

#if WITH_EDITOR
	// The editor should reset NextKeyInt, so that clearing and generating new content doesn't rack up the key endlessly.
	if (GEngine->IsEditor())
	{
		NextKeyInt = 100;
	}

	// See Footnote1
#endif
}

FEntryKey UFaerieItemStorage::MoveStack(UFaerieItemStorage* ToStorage, const FInventoryKey Key, const int32 Amount)
{
	if (!IsValid(ToStorage) ||
		!IsValidKey(Key.EntryKey) ||
		!Faerie::ItemData::IsValidStack(Amount) ||
		!CanRemoveStack(Key, FFaerieItemStorageEvents::Get().Removal_Moving))
	{
		return FEntryKey::InvalidKey;
	}

	const int32 Stack = Amount == Faerie::ItemData::UnlimitedStack ? Faerie::ItemData::UnlimitedStack : Amount;

	const FConstStructView EntryView = GetEntryView(Key.EntryKey);
	if (!ensure(EntryView.IsValid()))
	{
		return false;
	}

	const FInventoryEntry& Entry = EntryView.Get<const FInventoryEntry>();

	FFaerieItemStack ItemStack;
	ItemStack.Item = Entry.ItemObject;
	ItemStack.Copies = Stack == Faerie::ItemData::UnlimitedStack ? Entry.StackSum() : FMath::Min(Stack, Entry.StackSum());

	if (!ToStorage->CanAddStack(ItemStack))
	{
		return FEntryKey::InvalidKey;
	}

	const Faerie::Inventory::FEventLog Removed = RemoveFromStackImpl(Key, Stack, FFaerieItemStorageEvents::Get().Removal_Moving);

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

	const FConstStructView EntryView = GetEntryView(Key);
	if (!ensure(EntryView.IsValid()))
	{
		return false;
	}

	if (!ToStorage->CanAddStack(EntryView.Get<const FInventoryEntry>().ToItemStackView()))
	{
		return FEntryKey::InvalidKey;
	}

	auto&& Result = RemoveFromEntryImpl(Key, Faerie::ItemData::UnlimitedStack, FFaerieItemStorageEvents::Get().Removal_Moving);

	if (!ensure(Result.Success))
	{
		return FEntryKey::InvalidKey;
	}

	return ToStorage->AddEntryFromStackImpl({const_cast<UFaerieItem*>(Result.Item.Get()), Result.Amount}).EntryTouched;
}

void UFaerieItemStorage::Dump(UFaerieItemStorage* ToStorage)
{
	for (const FKeyedInventoryEntry& Element : EntryMap)
	{
		MoveEntry(ToStorage, Element.Key);
	}
}


/*
 * Footnote1: You might think that even at runtime we could reset the key during Clear, since all items are removed,
 * and therefor no entries exist, making 100 a valid starting point again, *except* that other entities might still
 * be holding onto FEntryKeys, which could be cached in at some point later when potentially the entry is once more
 * valid, but with a completely different item. So during runtime, the Key must always increment.
 * The exception to this is a full shutdown & reload. FEntryKeys should not *ever* be serialized to disk, so during
 * PostLoad it is completely fine to reset the key to 100 if EntryMap is empty.
 */