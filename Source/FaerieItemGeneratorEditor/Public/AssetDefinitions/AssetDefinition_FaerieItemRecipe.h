// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "AssetDefinitionDefault.h"
#include "FaerieDataSystemGlobals.h"
#include "FaerieItemRecipe.h"
#include "AssetDefinition_FaerieItemRecipe.generated.h"

/**
 *
 */
UCLASS()
class FAERIEITEMGENERATOREDITOR_API UAssetDefinition_FaerieItemRecipe : public UAssetDefinitionDefault
{
	GENERATED_BODY()

public:
	// UAssetDefinition Begin
	virtual FText GetAssetDisplayName() const override
	{
		return NSLOCTEXT("AssetDefinitions", "FaerieItemRecipe", "Faerie Item Recipe");
	}
	virtual FText GetAssetDescription(const FAssetData& AssetData) const override
	{
		return NSLOCTEXT("AssetDefinitions", "FaerieItemRecipe_Desc", "A simple crafted item");
	}
	virtual FLinearColor GetAssetColor() const override { return FLinearColor(FColor(127, 0, 155)); } // Nice purple color
	virtual TSoftClassPtr<UObject> GetAssetClass() const override { return UFaerieItemRecipe::StaticClass(); }
	virtual TConstArrayView<FAssetCategoryPath> GetAssetCategories() const override
	{
		return Faerie::Editor::DefaultCategories;
	}
	// UAssetDefinition End
};