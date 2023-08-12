// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "ItemUpgradeConfig.h"

DEFINE_LOG_CATEGORY(LogItemUpgradeConfig)

#if WITH_EDITOR

#define LOCTEXT_NAMESPACE "ItemUpgradeConfigValidation"

EDataValidationResult UItemUpgradeConfig::IsDataValid(TArray<FText>& ValidationErrors)
{
	FText ErrorMessage;
	bool HasError = false;

	if (!Mutator)
	{
		ErrorMessage = LOCTEXT("MutatorNotValid", "Mutators is invalid.");
		ValidationErrors.Add(ErrorMessage);
		HasError = true;
	}

	return HasError ? EDataValidationResult::Invalid : EDataValidationResult::Valid;
}

#undef LOCTEXT_NAMESPACE

#endif

FConstStructView UItemUpgradeConfig::GetCraftingSlots() const
{
	return UFaerieItemSlotLibrary::GetCraftingSlotsFromObject(Mutator);
}