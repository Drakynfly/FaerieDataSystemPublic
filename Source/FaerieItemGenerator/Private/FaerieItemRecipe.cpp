// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "FaerieItemRecipe.h"

FFaerieCraftingSlotsView UFaerieItemRecipe::GetCraftingSlots() const
{
	return FFaerieCraftingSlotsView::Make(CraftingSlots);
}