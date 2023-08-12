// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "Factories/Factory.h"
#include "FaerieItemFactories.generated.h"

UCLASS()
class FAERIEITEMDATAEDITOR_API UFaerieItemAsset_Factory : public UFactory
{
    GENERATED_BODY()

public:
    UFaerieItemAsset_Factory(const FObjectInitializer& ObjectInitializer);

protected:
    virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
};

UCLASS()
class FAERIEITEMDATAEDITOR_API UFaerieItemTemplate_Factory : public UFactory
{
    GENERATED_BODY()

public:
    UFaerieItemTemplate_Factory(const FObjectInitializer& ObjectInitializer);

protected:
    virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
};