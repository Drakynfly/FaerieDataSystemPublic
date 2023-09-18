// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "Extensions/InventoryItemLimitExtension.h"

#include "FaerieItemStorage.h"

void UInventoryItemLimitExtension::InitializeExtension(const UFaerieItemContainerBase* Container)
{
	if (!ensure(IsValid(Container))) return;

	Container->ForEachKey(
		[this, Container](const FEntryKey Key)
		{
			UpdateCacheForEntry(Container, Key);
		});
}

void UInventoryItemLimitExtension::DeinitializeExtension(const UFaerieItemContainerBase* Container)
{
	if (!ensure(IsValid(Container))) return;

	Container->ForEachKey(
		[this](const FEntryKey Key)
		{
			int32 Value = 0;
			StackAmountCache.RemoveAndCopyValue(Key, Value);
			CurrentTotalItemCopies -= Value;
		});
}

EEventExtensionResponse UInventoryItemLimitExtension::AllowsAddition(const UFaerieItemContainerBase* Container,
                                                                     const FFaerieItemStackView Stack)
{
	if (!CanContain(Stack))
	{
		// @todo this is a routine call, which is normal to fail. Why log failure at all?
		UE_LOG(LogTemp, VeryVerbose,
			TEXT("AllowsAddition: Cannot add Stack (Copies: %i)"),
			Stack.Copies);

		return EEventExtensionResponse::Disallowed;
	}

	return EEventExtensionResponse::Allowed;
}

void UInventoryItemLimitExtension::PostAddition(const UFaerieItemContainerBase* Container, const Faerie::Inventory::FEventLog& Event)
{
	UpdateCacheForEntry(Container, Event.EntryTouched);
}

void UInventoryItemLimitExtension::PostRemoval(const UFaerieItemContainerBase* Container, const Faerie::Inventory::FEventLog& Event)
{
	UpdateCacheForEntry(Container, Event.EntryTouched);
}

void UInventoryItemLimitExtension::PostEntryChanged(const UFaerieItemContainerBase* Container, const FEntryKey Key)
{
	UpdateCacheForEntry(Container, Key);
}

int32 UInventoryItemLimitExtension::GetTotalItemCount() const
{
	return CurrentTotalItemCopies;
}

int32 UInventoryItemLimitExtension::GetRemainingStackCount() const
{
	if (MaxEntries <= 0)
	{
		return Faerie::ItemData::UnlimitedStack;
	}
	return MaxEntries - StackAmountCache.Num();
}

int32 UInventoryItemLimitExtension::GetRemainingTotalItemCount() const
{
	if (MaxTotalItemCopies <= 0)
	{
		return Faerie::ItemData::UnlimitedStack;
	}
	return MaxTotalItemCopies - CurrentTotalItemCopies;
}

bool UInventoryItemLimitExtension::CanContain(const FFaerieItemStackView Stack) const
{
	if (MaxEntries > 0)
	{
		if (StackAmountCache.Num() >= MaxEntries)
		{
			return false;
		}
	}

	if (MaxTotalItemCopies > 0)
	{
		if (CurrentTotalItemCopies + Stack.Copies > MaxTotalItemCopies)
		{
			return false;
		}
	}

	return true;
}

void UInventoryItemLimitExtension::UpdateCacheForEntry(const UFaerieItemContainerBase* Container, const FEntryKey Key)
{
	if (!ensure(IsValid(Container))) return;

	const int32* PrevStackCache = StackAmountCache.Find(Key);

	if (!Container->IsValidKey(Key))
	{
		if (PrevStackCache)
		{
			CurrentTotalItemCopies -= *PrevStackCache;
			StackAmountCache.Remove(Key);
		}
		return;
	}

	const int32 StackAtKey = Container->GetStack(Key);

	int32 Diff = StackAtKey;

	if (PrevStackCache)
	{
		Diff -= *PrevStackCache;
	}

	StackAmountCache.Add(Key, StackAtKey);
	CurrentTotalItemCopies += Diff;
}