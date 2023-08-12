// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "UI/InventorySortRule.h"

bool UInventorySortRule::Exec(const UFaerieItemDataProxyBase* A, const UFaerieItemDataProxyBase* B) const
{
	const UInventoryEntryProxyBase* EntryA = Cast<UInventoryEntryProxyBase>(A);
	const UInventoryEntryProxyBase* EntryB = Cast<UInventoryEntryProxyBase>(B);

	if (IsValid(EntryA) && IsValid(EntryB))
	{
		return Execute(EntryA, EntryB);
	}

	return false;
}

bool UInventorySortRule::Execute_Implementation(const UInventoryEntryProxyBase* A, const UInventoryEntryProxyBase* B) const
{
	checkf(0, TEXT("This should be overriden!"))
	return false;
}
