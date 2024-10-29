// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "FaerieDataSystemEditorModule.h"
#include "Modules/ModuleManager.h"

class FFaerieInventoryEditorModule : public IFaerieDataSystemEditorModuleBase
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
};