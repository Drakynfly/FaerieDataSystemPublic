// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "AssetDefinitionDefault.h"
#include "FaerieDataSystemGlobals.h"
#include "FaerieItemTemplate.h"
#include "AssetDefinition_FaerieItemTemplate.generated.h"

/**
 *
 */
UCLASS()
class FAERIEITEMDATAEDITOR_API UAssetDefinition_FaerieItemTemplate : public UAssetDefinitionDefault
{
	GENERATED_BODY()

public:
	// UAssetDefinition Begin
	virtual FText GetAssetDisplayName() const override
	{
		return NSLOCTEXT("AssetDefinitions", "FaerieItemTemplate", "Faerie Item Template");
	}
	virtual FText GetAssetDescription(const FAssetData& AssetData) const override
	{
		return NSLOCTEXT("AssetDefinitions", "FaerieItemTemplate_Desc", "An asset for an item pattern, that matches against faerie item proxies.");
	}
	virtual FLinearColor GetAssetColor() const override { return FLinearColor(FColor(11, 156, 132)); } // Terezi teal
	virtual TSoftClassPtr<UObject> GetAssetClass() const override { return UFaerieItemTemplate::StaticClass(); }
	virtual TConstArrayView<FAssetCategoryPath> GetAssetCategories() const override
	{
		return Faerie::Editor::DefaultCategories;
	}
	// UAssetDefinition End
};