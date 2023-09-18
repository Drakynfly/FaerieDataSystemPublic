// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "FaerieItemSlotUtils.h"

FConstStructView UFaerieItemSlotLibrary::GetCraftingSlotsFromObject(const UObject* Object)
{
	if (IsValid(Object) && Object->Implements<UFaerieItemSlotInterface>())
	{
		return Cast<IFaerieItemSlotInterface>(Object)->GetCraftingSlots();
	}
	return FConstStructView(FFaerieItemCraftingSlots::StaticStruct());
}

FConstStructView UFaerieItemSlotLibrary::GetCraftingSlotsFromInterface(const IFaerieItemSlotInterface* Interface)
{
	if (Interface != nullptr)
	{
		return Interface->GetCraftingSlots();
	}
	return FConstStructView(FFaerieItemCraftingSlots::StaticStruct());
}

void UFaerieItemSlotLibrary::GetCraftingSlots(const TScriptInterface<IFaerieItemSlotInterface> Interface, FFaerieItemCraftingSlots& Slots)
{
	Slots = FFaerieItemCraftingSlots();

	if (Interface.GetInterface())
	{
		if (const FConstStructView SlotsView = Interface->GetCraftingSlots();
			SlotsView.IsValid())
		{
			Slots = SlotsView.Get<const FFaerieItemCraftingSlots>();
		}
	}
}

void UFaerieItemSlotLibrary::GetCraftingSlots_Message(UObject* Object, FFaerieItemCraftingSlots& Slots)
{
	if (Object && Object->Implements<UFaerieItemSlotInterface>())
	{
		GetCraftingSlots(TScriptInterface<IFaerieItemSlotInterface>(Object), Slots);
	}
}

bool UFaerieItemSlotLibrary::IsSlotOptional(const TScriptInterface<IFaerieItemSlotInterface> Interface, const FFaerieItemSlotHandle& Name)
{
	if (Interface.GetInterface() == nullptr) return false;

	const FConstStructView SlotsView = GetCraftingSlotsFromInterface(Interface.GetInterface());
	return SlotsView.Get<const FFaerieItemCraftingSlots>().OptionalSlots.Contains(Name);
}

bool UFaerieItemSlotLibrary::FindSlot(const TScriptInterface<IFaerieItemSlotInterface> Interface,
									  const FFaerieItemSlotHandle& Name, UFaerieItemTemplate*& OutSlot)
{
	if (Interface.GetInterface() == nullptr) return false;

	const FConstStructView SlotsView = GetCraftingSlotsFromInterface(Interface.GetInterface());
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