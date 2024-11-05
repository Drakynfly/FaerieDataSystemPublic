// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "FaerieItemFactories.h"
#include "FaerieItemAsset.h"
#include "FaerieItemTemplate.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FaerieItemFactories)

// FAERIE ITEM ASSET
UFaerieItemAsset_Factory::UFaerieItemAsset_Factory(const FObjectInitializer& ObjectInitializer)
  : Super(ObjectInitializer)
{
    bEditAfterNew = true;
    bCreateNew = true;
    SupportedClass = UFaerieItemAsset::StaticClass();
}

UObject* UFaerieItemAsset_Factory::FactoryCreateNew(UClass* Class,
                                                   UObject* InParent,
                                                   const FName Name,
                                                   const EObjectFlags Flags,
                                                   UObject* Context,
                                                   FFeedbackContext* Warn)
{
    return NewObject<UFaerieItemAsset>(InParent, Class, Name, Flags | RF_Transactional, Context);
}

// FAERIE ITEM TEMPLATE
UFaerieItemTemplate_Factory::UFaerieItemTemplate_Factory(const FObjectInitializer& ObjectInitializer)
  : Super(ObjectInitializer)
{
    bEditAfterNew = true;
    bCreateNew = true;
    SupportedClass = UFaerieItemTemplate::StaticClass();
}

UObject* UFaerieItemTemplate_Factory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags,
    UObject* Context, FFeedbackContext* Warn)
{
    return NewObject<UFaerieItemTemplate>(InParent, Class, Name, Flags | RF_Transactional, Context);
}