// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "FaerieItemSlotInterface.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FaerieItemSlotInterface)

namespace Faerie::Crafting
{
	FFaerieCraftingSlotsView GetCraftingSlots(const IFaerieItemSlotInterface* Interface)
	{
		if (Interface != nullptr)
		{
			return Interface->GetCraftingSlots();
		}
		return FFaerieCraftingSlotsView(FFaerieItemCraftingSlots::StaticStruct());
	}

	bool IsSlotOptional(const IFaerieItemSlotInterface* Interface, const FFaerieItemSlotHandle& Name)
	{
		if (Interface == nullptr) return false;
		const FFaerieCraftingSlotsView SlotsView = GetCraftingSlots(Interface);
		return SlotsView.Get<const FFaerieItemCraftingSlots>().OptionalSlots.Contains(Name);
	}

	bool FindSlot(const IFaerieItemSlotInterface* Interface, const FFaerieItemSlotHandle& Name, UFaerieItemTemplate*& OutSlot)
	{
		if (Interface == nullptr) return false;

		const FFaerieCraftingSlotsView SlotsView = GetCraftingSlots(Interface);
		const FFaerieItemCraftingSlots& SlotsPtr = SlotsView.Get<const FFaerieItemCraftingSlots>();

		if (SlotsPtr.RequiredSlots.Contains(Name))
		{
			OutSlot = SlotsPtr.RequiredSlots[Name];
			return true;
		}

		if (SlotsPtr.OptionalSlots.Contains(Name))
		{
			OutSlot = SlotsPtr.OptionalSlots[Name];
			return true;
		}

		return false;
	}
}