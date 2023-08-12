// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "FaerieEquipmentModule.h"

#include "FaerieSlotTag.h"
#include "Developer/Settings/Public/ISettingsModule.h"

#define LOCTEXT_NAMESPACE "FaerieEquipmentModule"

FFaerieSlotTags FFaerieSlotTags::FaerieSlotTags;

void FFaerieEquipmentModule::StartupModule()
{
}

void FFaerieEquipmentModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FFaerieEquipmentModule, FaerieEquipment)