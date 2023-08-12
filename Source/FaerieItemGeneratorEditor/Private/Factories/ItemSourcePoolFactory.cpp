// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "FaerieDataSystemFactories.h"
#include "FaerieItemRecipe.h"
#include "ItemSourcePool.h"

// ITEM SOURCE POOL
UItemSourcePool_Factory::UItemSourcePool_Factory(const FObjectInitializer& ObjectInitializer)
  : Super(ObjectInitializer)
{
    bEditAfterNew = true;
    bCreateNew = true;
    SupportedClass = UItemSourcePool::StaticClass();
}

UObject* UItemSourcePool_Factory::FactoryCreateNew(UClass* Class,
                                                        UObject* InParent,
                                                        const FName Name,
                                                        const EObjectFlags Flags,
                                                        UObject* Context,
                                                        FFeedbackContext* Warn)
{
    return NewObject<UItemSourcePool>(InParent, Class, Name, Flags | RF_Transactional, Context);
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