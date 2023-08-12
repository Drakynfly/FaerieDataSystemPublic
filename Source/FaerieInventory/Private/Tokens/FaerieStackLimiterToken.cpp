// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "Tokens/FaerieStackLimiterToken.h"

/* static */ FInventoryStack UFaerieStackLimiterToken::GetItemStackLimit(const UFaerieItem* Item)
{
	if (!ensure(IsValid(Item)))
	{
		return FInventoryStack::EmptyStack;
	}

	if (auto&& Limiter = Item->GetToken<UFaerieStackLimiterToken>())
	{
		return Limiter->GetStackLimit();
	}

	// Enforce stack limit to 1, if item has potential to have variation between instances
	if (Item->IsDataMutable()) return 1;

	// If no stack limiter is present, and the item is immutable, it can always stack
	return FInventoryStack::UnlimitedStack;
}

FInventoryStack UFaerieStackLimiterToken::GetStackLimit() const
{
	// Enforce stack limit to 1, if item has potential to have variation between instances
	if (IsOuterItemMutable()) return 1;

	// Interpret a limit of 0 as no limit.
	if (Size == 0) return FInventoryStack::UnlimitedStack;

	// Otherwise, use authored value for stack size.
	return Size;
}