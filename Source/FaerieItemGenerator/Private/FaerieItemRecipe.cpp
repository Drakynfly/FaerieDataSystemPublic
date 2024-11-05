// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "FaerieItemRecipe.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FaerieItemRecipe)

FFaerieCraftingSlotsView UFaerieItemRecipe::GetCraftingSlots() const
{
	return FFaerieCraftingSlotsView::Make(CraftingSlots);
}