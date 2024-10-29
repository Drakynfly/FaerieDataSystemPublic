// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "CraftingLibrary.h"
#include "FaerieItemSlotInterface.h"
#include "ItemGeneratorConfig.h"

DEFINE_LOG_CATEGORY(LogCraftingLibrary);

UItemGenerationConfig* UCraftingLibrary::CreateGenerationDriver(const TArray<FWeightedDrop>& DropList, const FGeneratorAmountBase& Amount)
{
    UItemGenerationConfig* NewDriver = NewObject<UItemGenerationConfig>();
    NewDriver->DropPool.DropList = DropList;
    NewDriver->AmountResolver = TInstancedStruct<FGeneratorAmountBase>::Make(Amount);
    return NewDriver;
}

void UCraftingLibrary::GetCraftingSlots(const TScriptInterface<IFaerieItemSlotInterface> Interface, FFaerieItemCraftingSlots& Slots)
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

void UCraftingLibrary::GetCraftingSlots_Message(UObject* Object, FFaerieItemCraftingSlots& Slots)
{
    if (Object && Object->Implements<UFaerieItemSlotInterface>())
    {
        GetCraftingSlots(TScriptInterface<IFaerieItemSlotInterface>(Object), Slots);
    }
}

bool UCraftingLibrary::IsSlotOptional(const TScriptInterface<IFaerieItemSlotInterface> Interface, const FFaerieItemSlotHandle& Name)
{
    return Faerie::Crafting::IsSlotOptional(Interface.GetInterface(), Name);
}

bool UCraftingLibrary::FindSlot(const TScriptInterface<IFaerieItemSlotInterface> Interface,
                                      const FFaerieItemSlotHandle& Name, UFaerieItemTemplate*& OutSlot)
{
    return Faerie::Crafting::FindSlot(Interface.GetInterface(), Name, OutSlot);
}