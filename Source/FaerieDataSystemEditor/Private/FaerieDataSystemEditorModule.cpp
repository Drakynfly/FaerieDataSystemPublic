// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "FaerieDataSystemEditorModule.h"

#include "PropertyEditorModule.h"
#include "PropertyEditorDelegates.h"
#include "Modules/ModuleManager.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleRegistry.h"

DEFINE_LOG_CATEGORY(LogFaerieDataSystemEditorModule);

#define LOCTEXT_NAMESPACE "FaerieDataSystemEditorModule"

void IFaerieDataSystemEditorModuleBase::StartupModule()
{
}

void IFaerieDataSystemEditorModuleBase::ShutdownModule()
{
    if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
    {
        auto&& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

        // Unregister detail customizations
        for (auto&& Customization : DetailCustomizations)
        {
            PropertyModule.UnregisterCustomClassLayout(Customization);
        }

        // Unregister property customizations
        for (auto&& Customization : PropertyCustomizations)
        {
            PropertyModule.UnregisterCustomPropertyTypeLayout(Customization);
        }

        PropertyModule.NotifyCustomizationModuleChanged();
    }
}

void IFaerieDataSystemEditorModuleBase::RegisterDetailCustomizations(
    const TMap<FName, FOnGetDetailCustomizationInstance>& Customizations)
{
    auto&& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

    for (auto&& Element : Customizations)
    {
        PropertyModule.RegisterCustomClassLayout(Element.Key, Element.Value);
        DetailCustomizations.Add(Element.Key);
    }

    PropertyModule.NotifyCustomizationModuleChanged();
}

void IFaerieDataSystemEditorModuleBase::RegisterPropertyCustomizations(
    const TMap<FName, FOnGetPropertyTypeCustomizationInstance>& Customizations)
{
    auto&& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

    for (auto&& Element : Customizations)
    {
        PropertyModule.RegisterCustomPropertyTypeLayout(Element.Key, Element.Value);
        PropertyCustomizations.Add(Element.Key);
    }

    PropertyModule.NotifyCustomizationModuleChanged();
}

FFaerieDataSystemEditorModule::FFaerieDataSystemEditorModule()
{
    StyleSet = MakeShareable(new FSlateStyleSet("FaerieDataSystemStyle"));

    // Content path of this plugin
    const FString ContentDir = IPluginManager::Get().FindPlugin("FaerieDataSystem")->GetBaseDir();

    // The image we wish to load is located inside the Resources folder inside the Base Directory
    // so let's set the content dir to the base dir and manually switch to the Resources folder:
    StyleSet->SetContentRoot(ContentDir);

    const FVector2D Size128 = FVector2D(128.f, 128.f);
    const TCHAR* ExtPng = TEXT(".png");

    TMap<FString, FString> PathAssetPairs;
    PathAssetPairs.Add("IconDefault128", "FaeriePrimaryDataAsset");
    PathAssetPairs.Add("IconInventory128", "FaerieItemAsset");
    PathAssetPairs.Add("IconWeapon128", "FaerieWeaponAsset");
    PathAssetPairs.Add("IconTable128", "ItemSourcePool");

    FSlateImageBrush* ThumbnailBrush;

    for (auto&& i : PathAssetPairs)
    {
        // Create a brush from the icon
        // ReSharper disable once CppJoinDeclarationAndAssignment
        ThumbnailBrush = new FSlateImageBrush(StyleSet->RootToContentDir("Resources/" + i.Key, ExtPng), Size128);
        if (ThumbnailBrush)
        {
            // In order to bind the thumbnail to our class we need to type ClassThumbnail.X where X is the name of the C++ class of the asset
            StyleSet->Set(*("ClassThumbnail." + i.Value), ThumbnailBrush);
        }
    }
}

void FFaerieDataSystemEditorModule::StartupModule()
{
    FSlateStyleRegistry::RegisterSlateStyle(*StyleSet);
}

void FFaerieDataSystemEditorModule::ShutdownModule()
{
    FSlateStyleRegistry::UnRegisterSlateStyle(StyleSet->GetStyleSetName());
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FFaerieDataSystemEditorModule, FaerieDataSystemEditor);