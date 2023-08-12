// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "ItemCraftingConfig.h"
#include "FaerieItemRecipe.h"

DEFINE_LOG_CATEGORY(LogItemCraftingConfig)

#if WITH_EDITOR

#define LOCTEXT_NAMESPACE "ItemCraftingConfig_IsDataValid"

EDataValidationResult UItemCraftingConfig::IsDataValid(TArray<FText>& ValidationErrors)
{
	bool HasFoundError = false;

	if (!Recipe)
	{
		ValidationErrors.Add(LOCTEXT("InvalidSourceAsset", "Source Asset is invalid!"));
		HasFoundError = true;
	}

	if (HasFoundError)
	{
		return EDataValidationResult::Invalid;
	}
	return Super::IsDataValid(ValidationErrors);
}

#undef LOCTEXT_NAMESPACE

#endif

FConstStructView UItemCraftingConfig::GetCraftingSlots() const
{
	return UFaerieItemSlotLibrary::GetCraftingSlotsFromInterface(Recipe);
}