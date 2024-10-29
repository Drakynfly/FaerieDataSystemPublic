// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "ItemCraftingConfig.h"
#include "FaerieItemRecipe.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

DEFINE_LOG_CATEGORY(LogItemCraftingConfig)

#if WITH_EDITOR

#define LOCTEXT_NAMESPACE "ItemCraftingConfig_IsDataValid"

EDataValidationResult UItemCraftingConfig::IsDataValid(FDataValidationContext& Context) const
{
	if (!Recipe)
	{
		Context.AddError(LOCTEXT("InvalidSourceAsset", "Source Asset is invalid!"));
	}

	if (Context.GetNumErrors())
	{
		return EDataValidationResult::Invalid;
	}
	return Super::IsDataValid(Context);
}

#undef LOCTEXT_NAMESPACE

#endif

FFaerieCraftingSlotsView UItemCraftingConfig::GetCraftingSlots() const
{
	return Faerie::Crafting::GetCraftingSlots(Recipe);
}