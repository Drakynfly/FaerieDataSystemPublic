// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "CraftingLibrary.h"
#include "ItemGeneratorConfig.h"

DEFINE_LOG_CATEGORY(LogCraftingLibrary);

UItemGenerationConfig* UCraftingLibrary::CreateGenerationDriver(const TArray<FWeightedDrop>& DropList, const FGeneratorAmountBase& Amount)
{
    UItemGenerationConfig* NewDriver = NewObject<UItemGenerationConfig>();
    NewDriver->DropPool.DropList = DropList;
    NewDriver->AmountResolver = TInstancedStruct<FGeneratorAmountBase>::Make(Amount);
    return NewDriver;
}