// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "ItemUpgradeConfig.h"
#include "FaerieItemMutator.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

DEFINE_LOG_CATEGORY(LogItemUpgradeConfig)

#if WITH_EDITOR

#define LOCTEXT_NAMESPACE "ItemUpgradeConfigValidation"

EDataValidationResult UItemUpgradeConfig::IsDataValid(FDataValidationContext& Context) const
{
	if (!Mutator)
	{
		Context.AddError(LOCTEXT("MutatorNotValid", "Mutators is invalid."));
	}

	if (Context.GetNumErrors())
	{
		return EDataValidationResult::Invalid;
	}

	return Super::IsDataValid(Context);
}

#undef LOCTEXT_NAMESPACE

#endif

FConstStructView UItemUpgradeConfig::GetCraftingSlots() const
{
	return UFaerieItemSlotLibrary::GetCraftingSlotsFromObject(Mutator);
}