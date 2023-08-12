// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "InventoryContentStructsLibrary.h"

DEFINE_LOG_CATEGORY(LogInventoryLibrary);

FInventoryStack UInventoryContentStructsLibrary::EmptyStack()
{
	return FInventoryStack::EmptyStack;
}

FInventoryStack UInventoryContentStructsLibrary::UnlimitedStack()
{
	return FInventoryStack::UnlimitedStack;
}

bool UInventoryContentStructsLibrary::IsUnlimited(const FInventoryStack Stack)
{
	return Stack == FInventoryStack::UnlimitedStack;
}

bool UInventoryContentStructsLibrary::Equal_EntryEntry(const FInventoryEntry& A, const FInventoryEntry& B, const int32 Checks)
{
	return FInventoryEntry::IsEqualTo(A, B, static_cast<EEntryEquivelancyFlags>(Checks));
}

void UInventoryContentStructsLibrary::SortEntriesLastModified(TArray<FInventoryEntry>& Entries)
{
	Algo::Sort(Entries, [](const FInventoryEntry& A, const FInventoryEntry& B)
		{
			return A.ItemObject->GetLastModified() < B.ItemObject->GetLastModified();
		});
}
