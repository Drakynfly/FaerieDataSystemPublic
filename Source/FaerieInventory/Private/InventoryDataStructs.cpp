// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

// ReSharper disable CppMemberFunctionMayBeConst
#include "InventoryDataStructs.h"
#include "FaerieItemStorage.h"
#include "HAL/LowLevelMemStats.h"
#include "Tokens/FaerieStackLimiterToken.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(InventoryDataStructs)

DEFINE_LOG_CATEGORY(LogInventoryStructs)

DECLARE_LLM_MEMORY_STAT(TEXT("ItemStorage"), STAT_StorageLLM, STATGROUP_LLMFULL);
DECLARE_LLM_MEMORY_STAT(TEXT("ItemStorage"), STAT_StorageSummaryLLM, STATGROUP_LLM);
LLM_DEFINE_TAG(ItemStorage, NAME_None, NAME_None, GET_STATFNAME(STAT_StorageLLM), GET_STATFNAME(STAT_StorageSummaryLLM));

FEntryKey FEntryKey::InvalidKey;

int32 FInventoryEntry::GetStack(const FStackKey& Key) const
{
	if (auto&& KeyedStack = Stacks.FindByKey(Key))
	{
		return KeyedStack->Stack;
	}
	return 0;
}

TArray<FStackKey> FInventoryEntry::CopyKeys() const
{
	TArray<FStackKey> Out;
	Algo::Transform(Stacks, Out, &FKeyedStack::Key);
	return Out;
}

int32 FInventoryEntry::StackSum() const
{
	int32 Out = 0;

	for (auto&& KeyedStack : Stacks)
	{
		Out += KeyedStack.Stack;
	}

	return Out;
}

void FInventoryEntry::SetStack(const FStackKey& Key, const int32 Stack)
{
	if (Stack <= 0)
	{
		if (const int32 StackIndex = Algo::BinarySearchBy(Stacks, Key, &FKeyedStack::Key);
			Stacks.IsValidIndex(StackIndex))
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

int32 FInventoryEntry::AddToAnyStack(int32 Stack, TArray<FStackKey>* OutAddedKeys)
{
	// Fill existing stacks first
	for (auto& KeyedStack : Stacks)
	{
		if (Limit == Faerie::ItemData::UnlimitedStack)
		{
			KeyedStack.Stack += Stack;
			Stack = 0;
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
	if (Stack == 0)
	{
		return Stack;
	}

	TArray<FStackKey> AddedStacks;

	if (Limit == Faerie::ItemData::UnlimitedStack)
	{
		const FStackKey NewKey = AddedStacks.Add_GetRef(KeyGen.NextKey());
		Stacks.Add({NewKey, Stack});
	}
	else
	{
		// Split the incoming stack into as many more as are required
		while (Stack > 0)
		{
			const FStackKey NewKey = AddedStacks.Add_GetRef(KeyGen.NextKey());
			const int32 NewStack = FMath::Min(Stack, Limit);
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

int32 FInventoryEntry::RemoveFromAnyStack(int32 Amount, TArray<FStackKey>* OutAllModifiedKeys, TArray<FStackKey>* OutRemovedKeys)
{
	TArray<FStackKey> RemovedStacks;

	// Remove from tail stack first
	for (int32 i = Stacks.Num() - 1; i >= 0; --i)
	{
		if (FKeyedStack& KeyedStack = Stacks[i];
			Amount >= KeyedStack.Stack)
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
	// No item, obviously invalid
	if (!ItemObject) return false;

	// No stacks, invalid
	if (Stacks.IsEmpty()) return false;

	// Invalid limit
	if (!Faerie::ItemData::IsValidStack(Limit)) return false;

	// Check that each stack is valid
	for (auto&& Element : Stacks)
	{
		if (!Element.Key.IsValid() || !Faerie::ItemData::IsValidStack(Element.Stack))
		{
			return false;
		}
	}

	// Everything is good
	return true;
}

FFaerieItemStackView FInventoryEntry::ToItemStackView() const
{
	FFaerieItemStackView Stack;
	Stack.Item = ItemObject;
	Stack.Copies = StackSum();
	return Stack;
}

void FInventoryEntry::PostSerialize(const FArchive& Ar)
{
	if (Ar.IsLoading())
	{
		if (!Stacks.IsEmpty())
		{
			KeyGen.SetPosition(Stacks.Last().Key);
		}
	}
}

bool FInventoryEntry::IsEqualTo(const FInventoryEntry& A, const FInventoryEntry& B, const EEntryEquivalencyFlags CheckFlags)
{
#define TEST_FLAG(Flag, Test)\
	if (EnumHasAnyFlags(CheckFlags, EEntryEquivalencyFlags::Test_##Flag)) if (!(Test)) return false;

	TEST_FLAG(Limit, A.Limit == B.Limit);
	TEST_FLAG(StackSum, A.StackSum() == B.StackSum());
	TEST_FLAG(ItemData, A.ItemObject->CompareWith(B.ItemObject));

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

FKeyedInventoryEntry& FInventoryContent::Append(const FEntryKey Key, const FInventoryEntry& Entry)
{
	check(Key.IsValid());

	LLM_SCOPE_BYTAG(ItemStorage);

	// Quick validation that Key *should* be stuck at the end of the array.
	if (!Entries.IsEmpty())
	{
		checkf(Entries.Last().Key < Key,
			TEXT("If this is hit, then Key is not sequential and Append was not safe to use. Either use a validated Key, or use FInventoryContent::Insert"));
	}

	FKeyedInventoryEntry& NewItemRef = Entries.Add_GetRef({Key, Entry});
	MarkItemDirty(NewItemRef);
	PostEntryReplicatedAdd(NewItemRef);
	return NewItemRef;
}

FKeyedInventoryEntry& FInventoryContent::AppendUnsafe(FEntryKey Key, const FInventoryEntry& Entry)
{
	check(Key.IsValid());

	LLM_SCOPE_BYTAG(ItemStorage);

	FKeyedInventoryEntry& NewItemRef = Entries.Add_GetRef({Key, Entry});
	MarkItemDirty(NewItemRef);
	PostEntryReplicatedAdd(NewItemRef);
	return NewItemRef;
}

void FInventoryContent::Insert(const FEntryKey Key, const FInventoryEntry& Entry)
{
	check(Key.IsValid());

	LLM_SCOPE_BYTAG(ItemStorage);

	FKeyedInventoryEntry& NewEntry = BSOA::Insert({Key, Entry});

	PostEntryReplicatedAdd(NewEntry);
	MarkItemDirty(NewEntry);
}

void FInventoryContent::Remove(const FEntryKey Key)
{
	if (BSOA::Remove(Key,
		[this](const FKeyedInventoryEntry& Entry)
		{
			// Notify owning server of this removal.
			PreEntryReplicatedRemove(Entry);
		}))
	{
		// Notify clients of this removal.
		MarkArrayDirty();
	}
}

FInventoryContent::FScopedItemHandle::~FScopedItemHandle()
{
	// Propagate change to client
	Source.MarkItemDirty(Handle);

	// Broadcast change on server
	Source.PostEntryReplicatedChange(Handle);
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
	void BreakKeyedEntriesIntoInventoryKeys(const TArray<FKeyedInventoryEntry>& Entries, TArray<FInventoryKey>& OutKeys)
	{
		OutKeys.Empty(Entries.Num());

		for (auto&& Entry : Entries)
		{
			for (auto&& Stack : Entry.Value.Stacks)
			{
				OutKeys.Add({Entry.Key, Stack.Key});
			}
		}
	}
}