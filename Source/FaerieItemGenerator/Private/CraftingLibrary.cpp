// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "CraftingLibrary.h"
#include "ItemGeneratorConfig.h"

DEFINE_LOG_CATEGORY(LogCraftingLibrary);

UItemGenerationConfig* UCraftingLibrary::CreateGenerationDriver(const FFaerieWeightedDropPool& Pool, const FGeneratorAmountBase& Amount)
{
    UItemGenerationConfig* NewDriver = NewObject<UItemGenerationConfig>();
    NewDriver->DropPool = Pool;
    NewDriver->AmountResolver = TInstancedStruct<FGeneratorAmountBase>::Make(Amount);
    return NewDriver;
}