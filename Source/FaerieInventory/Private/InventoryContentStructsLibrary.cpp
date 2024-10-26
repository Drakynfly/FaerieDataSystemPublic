// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "InventoryContentStructsLibrary.h"

DEFINE_LOG_CATEGORY(LogInventoryLibrary);

int32 UInventoryContentStructsLibrary::UnlimitedStack()
{
	return Faerie::ItemData::UnlimitedStack;
}

bool UInventoryContentStructsLibrary::IsUnlimited(const int32 Stack)
{
	return Stack == Faerie::ItemData::UnlimitedStack;
}

bool UInventoryContentStructsLibrary::Equal_EntryEntry(const FInventoryEntry& A, const FInventoryEntry& B, const int32 Checks)
{
	return FInventoryEntry::IsEqualTo(A, B, static_cast<EEntryEquivalencyFlags>(Checks));
}

void UInventoryContentStructsLibrary::SortEntriesLastModified(TArray<FInventoryEntry>& Entries)
{
	Algo::Sort(Entries, [](const FInventoryEntry& A, const FInventoryEntry& B)
		{
			return A.ItemObject->GetLastModified() < B.ItemObject->GetLastModified();
		});
}

int32 UInventoryContentStructsLibrary::GetStackSum(const FInventoryEntry& Entry)
{
	return Entry.StackSum();
}

FFaerieItemStackView UInventoryContentStructsLibrary::EntryToStackView(const FInventoryEntry& Entry)
{
	return Entry.ToItemStackView();
}