// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

struct FInventoryEntry;

namespace Faerie::Inventory
{
	/* Sorts an array of inventory entries by their modified data. */
	FAERIEINVENTORY_API void SortEntriesLastModified(TArray<FInventoryEntry>& Entries);
};