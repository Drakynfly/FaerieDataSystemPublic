// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "FaerieDataSystemFactories.h"
#include "FaerieItemRecipe.h"
#include "FaerieItemPool.h"

// FAERIE ITEM POOL
UFaerieItemPool_Factory::UFaerieItemPool_Factory(const FObjectInitializer& ObjectInitializer)
  : Super(ObjectInitializer)
{
    bEditAfterNew = true;
    bCreateNew = true;
    SupportedClass = UFaerieItemPool::StaticClass();
}

UObject* UFaerieItemPool_Factory::FactoryCreateNew(UClass* Class,
                                                        UObject* InParent,
                                                        const FName Name,
                                                        const EObjectFlags Flags,
                                                        UObject* Context,
                                                        FFeedbackContext* Warn)
{
    return NewObject<UFaerieItemPool>(InParent, Class, Name, Flags | RF_Transactional, Context);
}


// FAERIE ITEM RECIPE
UFaerieItemRecipe_Factory::UFaerieItemRecipe_Factory(const FObjectInitializer& ObjectInitializer)
  : Super(ObjectInitializer)
{
    bEditAfterNew = true;
    bCreateNew = true;
    SupportedClass = UFaerieItemRecipe::StaticClass();
}

UObject* UFaerieItemRecipe_Factory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags,
    UObject* Context, FFeedbackContext* Warn)
{
    return NewObject<UFaerieItemRecipe>(InParent, Class, Name, Flags | RF_Transactional, Context);
}