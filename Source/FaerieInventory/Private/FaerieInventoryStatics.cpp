// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "FaerieInventoryStatics.h"
#include "InventoryDataStructs.h"

namespace Faerie::Inventory
{
	void SortEntriesLastModified(TArray<FInventoryEntry>& Entries)
	{
		Algo::Sort(Entries,
			[](const FInventoryEntry& A, const FInventoryEntry& B)
			{
				if (!A.ItemObject || !B.ItemObject) return false;
				return A.ItemObject->GetLastModified() < B.ItemObject->GetLastModified();
			});
	}
}