// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

// ReSharper disable CppMemberFunctionMayBeConst
#include "InventoryDataStructs.h"

#include "FaerieItemDataLibrary.h"
#include "FaerieItemStorage.h"
#include "HAL/LowLevelMemStats.h"

DEFINE_LOG_CATEGORY(LogInventoryStructs)

DECLARE_LLM_MEMORY_STAT(TEXT("ItemStorage"), STAT_StorageLLM, STATGROUP_LLMFULL);
DECLARE_LLM_MEMORY_STAT(TEXT("ItemStorage"), STAT_StorageSummaryLLM, STATGROUP_LLM);
LLM_DEFINE_TAG(ItemStorage, NAME_None, NAME_None, GET_STATFNAME(STAT_StorageLLM), GET_STATFNAME(STAT_StorageSummaryLLM));

FEntryKey FEntryKey::InvalidKey;

int32 FInventoryStack::EmptyStackNum = 0;
int32 FInventoryStack::UnlimitedNum = -1;
FInventoryStack FInventoryStack::EmptyStack(0);
FInventoryStack FInventoryStack::UnlimitedStack = MakeUnlimitedStack();

bool FInventoryStack::IsValid() const
{
	return Amount > 0 || Amount == UnlimitedNum;
}

FInventoryStack FInventoryStack::MakeUnlimitedStack()
{
	FInventoryStack Stack;
	Stack.Amount = -1;
	return Stack;
}

FInventoryStack FInventoryEntry::GetStack(const FStackKey& Key) const
{
	if (auto&& KeyedStack = Stacks.FindByKey(Key))
	{
		return KeyedStack->Stack;
	}
	return FInventoryStack::EmptyStack;
}

TArray<FStackKey> FInventoryEntry::GetKeys() const
{
	TArray<FStackKey> Out;

	for (auto&& KeyedStack : Stacks)
	{
		Out.Add(KeyedStack.Key);
	}

	return Out;
}

FInventoryStack FInventoryEntry::StackSum() const
{
	FInventoryStack Out = 0;

	for (auto&& KeyedStack : Stacks)
	{
		Out += KeyedStack.Stack;
	}

	return Out;
}

void FInventoryEntry::Set(const FStackKey& Key, const FInventoryStack Stack)
{
	if (Stack == FInventoryStack::EmptyStack)
	{
		const int32 StackIndex = Algo::BinarySearchBy(Stacks, Key, &FKeyedStack::Key, FStackKey::FCompare());

		if (Stacks.IsValidIndex(StackIndex))
		{
			Stacks.RemoveAt(StackIndex);
		}
		return;
	}

	if (auto&& KeyedStack = Stacks.FindByKey(Key))
	{
		KeyedStack->Stack = Stack;
	}
	else
	{
		Stacks.Add({Key, Stack});
	}
}

FInventoryStack FInventoryEntry::AddToAnyStack(FInventoryStack Stack, TArray<FStackKey>* OutAddedKeys)
{
	// Fill existing stacks first
	for (auto& KeyedStack : Stacks)
	{
		if (Limit == FInventoryStack::UnlimitedStack)
		{
			KeyedStack.Stack += Stack;
			Stack -= Stack;
			break;
		}

		if (KeyedStack.Stack < Limit)
		{
			auto&& AddToStack = Limit - KeyedStack.Stack;
			Stack -= AddToStack;
			KeyedStack.Stack += AddToStack;
		}
	}

	// We have dispersed the incoming stack among existing ones.
	if (Stack == FInventoryStack::EmptyStack)
	{
		return Stack;
	}

	TArray<FStackKey> AddedStacks;

	if (Limit == FInventoryStack::UnlimitedStack)
	{
		const FStackKey NewKey = AddedStacks.Add_GetRef(StackCount++);
		Stacks.Add({NewKey, Stack});
	}
	else
	{
		// Split the incoming stack into as many more as are required
		while (Stack > 0)
		{
			const FStackKey NewKey = AddedStacks.Add_GetRef(StackCount++);
			const FInventoryStack NewStack = FMath::Min(Stack, Limit);
			Stack -= NewStack;
			Stacks.Add({NewKey, NewStack});
		}
	}

	if (OutAddedKeys)
	{
		*OutAddedKeys = AddedStacks;
	}

	return Stack;
}

FInventoryStack FInventoryEntry::RemoveFromAnyStack(FInventoryStack Amount, TArray<FStackKey>* OutAllModifiedKeys, TArray<FStackKey>* OutRemovedKeys)
{
	TArray<FStackKey> RemovedStacks;

	// Remove from tail stack first
	for (int32 i = Stacks.Num() - 1; i >= 0; --i)
	{
		FKeyedStack& KeyedStack = Stacks[i];

		if (Amount >= KeyedStack.Stack)
		{
			RemovedStacks.Add(KeyedStack.Key);
			Amount -= KeyedStack.Stack;
			Stacks.Remove(KeyedStack);

			if (Amount <= 0)
			{
				break;
			}
		}
		else
		{
			KeyedStack.Stack -= Amount;
			if (OutAllModifiedKeys)
			{
				OutAllModifiedKeys->Add(KeyedStack.Key);
				OutAllModifiedKeys->Append(RemovedStacks);
			}
			break;
		}
	}

	if (OutRemovedKeys)
	{
		*OutRemovedKeys = RemovedStacks;
	}

	return Amount; // Return the remainder, if we didn't remove it all.
}

bool FInventoryEntry::IsValid() const
{
	if (!ItemObject) return false;

	if (Stacks.IsEmpty()) return false;

	if (!Limit.IsValid()) return false;

	for (auto&& Stack : Stacks)
	{
		if (!Stack.Key.IsValid() || !Stack.Stack.IsValid())
		{
			return false;
		}
	}

	return true;
}

FFaerieItemStack FInventoryEntry::ToItemStack() const
{
	FFaerieItemStack Stack;
	Stack.Item = ItemObject;
	Stack.Copies = StackSum().GetAmount();
	return Stack;
}

bool FInventoryEntry::IsEqualTo(const FInventoryEntry& A, const FInventoryEntry& B, const EEntryEquivelancyFlags CheckFlags)
{
#define TEST_FLAG(Flag, Test)\
	if (EnumHasAnyFlags(CheckFlags, EEntryEquivelancyFlags::##Flag)) if (!(Test)) return false;

	TEST_FLAG(ItemData, UFaerieItemDataLibrary::Equal_ItemData(A.ItemObject, B.ItemObject));
	TEST_FLAG(StackSum, A.StackSum() == B.StackSum());
	TEST_FLAG(Limit, A.Limit == B.Limit);

#undef TEST_FLAG

	return true;
}

void FKeyedInventoryEntry::PreReplicatedRemove(const FInventoryContent& InArraySerializer)
{
	InArraySerializer.PreEntryReplicatedRemove(*this);
}

void FKeyedInventoryEntry::PostReplicatedAdd(const FInventoryContent& InArraySerializer)
{
	InArraySerializer.PostEntryReplicatedAdd(*this);
}

void FKeyedInventoryEntry::PostReplicatedChange(const FInventoryContent& InArraySerializer)
{
	InArraySerializer.PostEntryReplicatedChange(*this);
}

int32 FInventoryContent::IndexOf(const FEntryKey Key) const
{
	// Search for Key in the Items. Since those do not share Type, a projection is provided, in the form of the address
	// of the appropriate member to compare against. Finally the custom Predicate is provided.
	return Algo::BinarySearchBy(Items, Key, &FKeyedInventoryEntry::Key, FEntryKey::FCompare());
}

bool FInventoryContent::Contains(const FEntryKey Key) const
{
	return IndexOf(Key) != INDEX_NONE;
}

const FInventoryEntry* FInventoryContent::Find(const FEntryKey Key) const
{
	const int32 Index = IndexOf(Key);

	if (Index != INDEX_NONE)
	{
		return &Items[Index].Entry;
	}
	return nullptr;
}

const FInventoryEntry& FInventoryContent::operator[](const FEntryKey Key) const
{
	return Items[IndexOf(Key)].Entry;
}

FEntryKey FInventoryContent::GetKeyAt(const int32 Index) const
{
	if (!Items.IsValidIndex(Index))
	{
		return FEntryKey();
	}

	return Items[Index].Key;
}

FKeyedInventoryEntry& FInventoryContent::Append(const FEntryKey Key, const FInventoryEntry& Entry)
{
	check(Key.IsValid());

	LLM_SCOPE_BYTAG(ItemStorage);

	// Quick validation that Key *should* be stuck at the end of the array.
	if (!Items.IsEmpty())
	{
		checkf(FEntryKey::FCompare()(Items.Last().Key, Key),
			TEXT("If this is hit, then Key is not sequential and Append was not safe to use. Either use a validated Key, or use FInventoryContent::Insert"));
	}

	FKeyedInventoryEntry& NewItemRef = Items.Add_GetRef({Key, Entry});
	MarkItemDirty(NewItemRef);
	ChangeListener->PostContentAdded(NewItemRef);
	return NewItemRef;
}

void FInventoryContent::Insert(const FEntryKey Key, const FInventoryEntry& Entry)
{
	check(Key.IsValid());

	LLM_SCOPE_BYTAG(ItemStorage);

	// Find the index of either ahead of where Key currently is, or where it should be inserted if it isn't present.
	const int32 NextIndex = Algo::UpperBoundBy(Items, Key, &FKeyedInventoryEntry::Key, FEntryKey::FCompare());

	if (NextIndex < Items.Num())
	{
		// Check if the index-1 is our key, and overwrite the data there if so.
		FKeyedInventoryEntry& CurrentEntry = Items[NextIndex-1];

		if (CurrentEntry.Key == Key)
		{
			CurrentEntry.Entry = Entry;
			ChangeListener->PostContentChanged(CurrentEntry);
			MarkItemDirty(CurrentEntry);
		}
	}
	// Otherwise, we were given a key not present and we should insert the Entry at the Index.
	else
	{
		FKeyedInventoryEntry& NewItemRef = Items.Insert_GetRef({Key, Entry}, NextIndex);
		MarkItemDirty(NewItemRef);
		ChangeListener->PostContentAdded(NewItemRef);
	}
}

void FInventoryContent::Sort()
{
	struct FCompare
	{
		bool operator()(const FKeyedInventoryEntry& A, const FKeyedInventoryEntry& B) const
		{
			return A.Key.Value() < B.Key.Value();
		}
	};

	Algo::Sort(Items, FCompare());
	// We don't bother to mark anything dirty right now. If clients become out-of-order, they can call this themselves.
	// If this causes problems, oops.
}

void FInventoryContent::Remove(const FEntryKey Key)
{
	const int32 Index = IndexOf(Key);
	if (Index != INDEX_NONE)
	{
		// Notify owning server of this removal.
		ChangeListener->PreContentRemoved(Items[Index]);
		Items.RemoveAt(Index);

		// Notify clients of this removal.
		MarkArrayDirty();
	}
}

FInventoryContent::FScopedItemHandle::~FScopedItemHandle()
{
	// Notify owning server of this change.
	Source.MarkItemDirty(Handle);

	// Notify clients of this change.
	Source.ChangeListener->PostContentChanged(Handle);
}

void FInventoryContent::PreEntryReplicatedRemove(const FKeyedInventoryEntry& Entry) const
{
	if (ChangeListener.IsValid())
	{
		ChangeListener->PreContentRemoved(Entry);
	}
}

void FInventoryContent::PostEntryReplicatedAdd(const FKeyedInventoryEntry& Entry) const
{
	if (ChangeListener.IsValid())
	{
		ChangeListener->PostContentAdded(Entry);
	}
}

void FInventoryContent::PostEntryReplicatedChange(const FKeyedInventoryEntry& Entry) const
{
	if (ChangeListener.IsValid())
	{
		ChangeListener->PostContentChanged(Entry);
	}
}

namespace Faerie::Inventory
{
	void Utils::BreakKeyedEntriesIntoInventoryKeys(const TArray<FKeyedInventoryEntry>& Entries,
		TArray<FInventoryKey>& OutKeys)
	{
		OutKeys.Empty(Entries.Num());
		for (auto&& Entry : Entries)
		{
			for (auto&& Stack : Entry.Entry.Stacks)
			{
				OutKeys.Add({Entry.Key, Stack.Key});
			}
		}
	}
}