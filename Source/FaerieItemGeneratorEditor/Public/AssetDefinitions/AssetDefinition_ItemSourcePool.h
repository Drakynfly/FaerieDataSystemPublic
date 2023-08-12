// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "AssetDefinitionDefault.h"
#include "FaerieDataSystemGlobals.h"
#include "ItemSourcePool.h"
#include "AssetDefinition_ItemSourcePool.generated.h"

/**
 *
 */
UCLASS()
class FAERIEITEMGENERATOREDITOR_API UAssetDefinition_ItemSourcePool : public UAssetDefinitionDefault
{
	GENERATED_BODY()

public:
	// UAssetDefinition Begin
	virtual FText GetAssetDisplayName() const override
	{
		return NSLOCTEXT("AssetDefinitions", "ItemSourcePool", "Faerie Item Pool");
	}
	virtual FText GetAssetDescription(const FAssetData& AssetData) const override
	{
		return NSLOCTEXT("AssetDefinitions", "ItemSourcePool_Desc", "An asset for making weighted drop tables.");
	}
	virtual FLinearColor GetAssetColor() const override { return FLinearColor(FColor(11, 156, 132)); } // Terezi teal
	virtual TSoftClassPtr<UObject> GetAssetClass() const override { return UItemSourcePool::StaticClass(); }
	virtual TConstArrayView<FAssetCategoryPath> GetAssetCategories() const override
	{
		static const TArray<FAssetCategoryPath> Categories = { Faerie::Editor::DefaultAssetCategory };
		return Categories;
	}
	// UAssetDefinition End
};