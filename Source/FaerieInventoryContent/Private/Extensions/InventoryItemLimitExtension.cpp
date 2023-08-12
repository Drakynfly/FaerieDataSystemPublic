// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "Extensions/InventoryItemLimitExtension.h"

#include "FaerieItemStorage.h"

void UInventoryItemLimitExtension::InitializeExtension(const UFaerieItemContainerBase* Container)
{
	if (!ensure(IsValid(Container))) return;

	Container->ForEachKey([this, Container](const FEntryKey Key)
		{
			UpdateCacheForEntry(Container, Key);
		});
}

void UInventoryItemLimitExtension::DeinitializeExtension(const UFaerieItemContainerBase* Container)
{
	if (!ensure(IsValid(Container))) return;

	Container->ForEachKey([this](const FEntryKey Key)
		{
			int32 Value = 0;
			StackAmountCache.RemoveAndCopyValue(Key, Value);
			CurrentTotalItemCount -= Value;
		});
}

EEventExtensionResponse UInventoryItemLimitExtension::AllowsAddition(const UFaerieItemContainerBase* Container,
                                                                     const FFaerieItemStackView Stack)
{
	if (!CanContain(Stack))
	{
		UE_LOG(LogTemp, Warning,
			TEXT("PreAddition: Cannot add Stack (Copies: %i)"),
			Stack.Copies);

		return EEventExtensionResponse::Disallowed;
	}

	return EEventExtensionResponse::Allowed;
}

void UInventoryItemLimitExtension::PostAddition(const UFaerieItemContainerBase* Container, const Faerie::FItemContainerEvent& Event)
{
	UpdateCacheForEntry(Container, Event.EntryTouched);
}

void UInventoryItemLimitExtension::PostRemoval(const UFaerieItemContainerBase* Container, const Faerie::FItemContainerEvent& Event)
{
	UpdateCacheForEntry(Container, Event.EntryTouched);
}

void UInventoryItemLimitExtension::PostEntryChanged(const UFaerieItemContainerBase* Container, const FEntryKey Key)
{
	UpdateCacheForEntry(Container, Key);
}

int32 UInventoryItemLimitExtension::GetTotalItemCount() const
{
	return CurrentTotalItemCount;
}

int32 UInventoryItemLimitExtension::GetRemainingStackCount() const
{
	if (MaxEntries == FInventoryStack::UnlimitedNum)
	{
		return MaxEntries;
	}
	return MaxEntries - StackAmountCache.Num();
}

int32 UInventoryItemLimitExtension::GetRemainingTotalItemCount() const
{
	if (MaxItems == 0)
	{
		return FInventoryStack::UnlimitedNum;
	}
	return MaxItems - CurrentTotalItemCount;
}

bool UInventoryItemLimitExtension::CanContain(const FFaerieItemStackView Stack) const
{
	if (StackAmountCache.Num() >= MaxEntries)
	{
		return false;
	}

	if (CurrentTotalItemCount + Stack.Copies > MaxItems)
	{
		return false;
	}

	return true;
}

void UInventoryItemLimitExtension::UpdateCacheForEntry(const UFaerieItemContainerBase* Container, const FEntryKey Key)
{
	if (!ensure(IsValid(Container))) return;

	const int32* PrevCache = StackAmountCache.Find(Key);

	if (!Container->IsValidKey(Key))
	{
		if (PrevCache)
		{
			CurrentTotalItemCount -= *PrevCache;
			StackAmountCache.Remove(Key);
		}
		return;
	}

	const int32 Total = Container->GetStack(Key).GetAmount();

	int32 Diff = Total;

	if (PrevCache)
	{
		Diff -= *PrevCache;
	}

	StackAmountCache.Add(Key, Total);
	CurrentTotalItemCount += Diff;
}
