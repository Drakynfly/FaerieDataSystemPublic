// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "Factories/Factory.h"
#include "FaerieDataSystemFactories.generated.h"

UCLASS()
class FAERIEITEMGENERATOREDITOR_API UItemSourcePool_Factory : public UFactory
{
    GENERATED_BODY()

public:
    UItemSourcePool_Factory(const FObjectInitializer& ObjectInitializer);

protected:
    virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
};

UCLASS()
class FAERIEITEMGENERATOREDITOR_API UFaerieItemRecipe_Factory : public UFactory
{
    GENERATED_BODY()

public:
    UFaerieItemRecipe_Factory(const FObjectInitializer& ObjectInitializer);

protected:
    virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
};