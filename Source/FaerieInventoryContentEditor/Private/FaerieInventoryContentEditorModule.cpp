// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "FaerieInventoryContentEditorModule.h"
#include "PropertyEditorModule.h"
#include "GameplayTagsEditorModule.h"
#include "SpatialStructs.h"
#include "Customizations/ItemCapacityCustomization.h"
#include "Customizations/ItemShapeCustomization.h"
#include "Extensions/InventoryMetadataExtension.h"
#include "Extensions/InventoryUserdataExtension.h"
#include "Tokens/CapacityTokenDetailsCustomization.h"
#include "Tokens/FaerieCapacityToken.h"

#define LOCTEXT_NAMESPACE "FaerieInventoryContentEditorModule"

void FFaerieInventoryContentEditorModule::StartupModule()
{
	IFaerieDataSystemEditorModuleBase::StartupModule();

	TMap<FName, FOnGetDetailCustomizationInstance> ClassCustomizations;

	ClassCustomizations.Add(UFaerieCapacityToken::StaticClass()->GetFName(),
	FOnGetDetailCustomizationInstance::CreateStatic(&FCapacityTokenDetailCustomization::MakeInstance));

	RegisterDetailCustomizations(ClassCustomizations);

	TMap<FName, FOnGetPropertyTypeCustomizationInstance> StructCustomizations;

	StructCustomizations.Add(FFaerieInventoryMetaTag::StaticStruct()->GetFName(),
	FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FGameplayTagCustomizationPublic::MakeInstance));
	StructCustomizations.Add(FFaerieInventoryUserTag::StaticStruct()->GetFName(),
	FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FGameplayTagCustomizationPublic::MakeInstance));
	StructCustomizations.Add(FWeightEditor::StaticStruct()->GetFName(),
	FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FInventoryWeightCustomization::MakeInstance));
	StructCustomizations.Add(FWeightEditor_Float::StaticStruct()->GetFName(),
	FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FInventoryWeightCustomization::MakeInstance));
	StructCustomizations.Add(FItemCapacity::StaticStruct()->GetFName(),
	FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FItemCapacityCustomization::MakeInstance));
	StructCustomizations.Add(FFaerieGridShape::StaticStruct()->GetFName(),
		FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FItemShapeCustomization::MakeInstance));

	RegisterPropertyCustomizations(StructCustomizations);
}

void FFaerieInventoryContentEditorModule::ShutdownModule()
{
	IFaerieDataSystemEditorModuleBase::ShutdownModule();
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FFaerieInventoryContentEditorModule, FaerieInventoryContentEditor)