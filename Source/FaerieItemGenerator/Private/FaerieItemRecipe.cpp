// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "FaerieItemRecipe.h"

FConstStructView UFaerieItemRecipe::GetCraftingSlots() const
{
	return FConstStructView::Make(CraftingSlots);
}