// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "AssetDefinitionDefault.h"
#include "FaerieDataSystemGlobals.h"
#include "FaerieItemPool.h"
#include "AssetDefinition_FaerieItemPool.generated.h"

/**
 *
 */
UCLASS()
class FAERIEITEMGENERATOREDITOR_API UAssetDefinition_FaerieItemPool : public UAssetDefinitionDefault
{
	GENERATED_BODY()

public:
	// UAssetDefinition Begin
	virtual FText GetAssetDisplayName() const override
	{
		return NSLOCTEXT("AssetDefinitions", "FaerieItemPool_Name", "Faerie Item Pool");
	}
	virtual FText GetAssetDescription(const FAssetData& AssetData) const override
	{
		return NSLOCTEXT("AssetDefinitions", "FaerieItemPool_Desc", "An asset for making weighted drop tables.");
	}
	virtual FLinearColor GetAssetColor() const override { return FLinearColor(FColor(11, 156, 132)); } // Terezi teal
	virtual TSoftClassPtr<UObject> GetAssetClass() const override { return UFaerieItemPool::StaticClass(); }
	virtual TConstArrayView<FAssetCategoryPath> GetAssetCategories() const override
	{
		return Faerie::Editor::DefaultCategories;
	}
	// UAssetDefinition End
};