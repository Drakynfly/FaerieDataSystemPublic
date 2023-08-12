// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "CraftingLibrary.h"
#include "ItemGeneratorConfig.h"

DEFINE_LOG_CATEGORY(LogCraftingLibrary);

UItemGenerationDriver* UCraftingLibrary::CreateGenerationDriver(UItemSourcePool* Pool, const FGeneratorAmountBase& Amount)
{
    UItemGenerationDriver* NewDriver = NewObject<UItemGenerationDriver>();
    NewDriver->Pool = Pool;
    NewDriver->AmountResolver = FInstancedStruct::Make(Amount);
    return NewDriver;
}