// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "ItemSourcePool.h"
#include "FaerieInfoObject.h"
#include "ItemInstancingContext_Crafting.h"

#include "Squirrel.h"
#include "UObject/ObjectSaveContext.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

UItemSourcePool::UItemSourcePool()
{
	TableInfo.ObjectName = FText::FromString("<Unnamed Table>");
}

void UItemSourcePool::PreSave(FObjectPreSaveContext SaveContext)
{
	Super::PreSave(SaveContext);

#if WITH_EDITOR
	SortTable();

	HasMutableDrops = false;

	for (auto&& Drop : DropList)
	{
		auto&& Source = Drop.Drop.Asset.Object.LoadSynchronous();

		if (auto&& Interface = Cast<IFaerieItemSource>(Source))
		{
			if (Interface->CanBeMutable())
			{
				HasMutableDrops = true;
				break;
			}
		}
	}
#endif
}

void UItemSourcePool::PostLoad()
{
	Super::PostLoad();
#if WITH_EDITOR
	CalculatePercentages();
#endif
}

#if WITH_EDITOR

#define LOCTEXT_NAMESPACE "ItemSourcePoolValidation"

EDataValidationResult UItemSourcePool::IsDataValid(FDataValidationContext& Context)
{
	FText ErrorMessage;
	bool HasError = false;

	TArray<FFaerieItemSourceObject> AssetList;

	for (const FWeightedDrop& Entry : DropList)
	{
		if (!Entry.Drop.IsValid())
		{
			ErrorMessage = LOCTEXT("DropTableInvalidAsset_Ref", "Invalid Asset Reference");
			Context.AddWarning(ErrorMessage);
			HasError = true;
		}
		else
		{
			if (AssetList.Contains(Entry.Drop.Asset))
			{
				ErrorMessage = LOCTEXT("DropTableInvalidAsset_Dup", "Asset already exists in table. Please only have one weight per asset.");
				Context.AddWarning(ErrorMessage);
				HasError = true;
			}
			else
			{
				AssetList.Add(Entry.Drop.Asset);
			}
		}
		if (Entry.Weight <= 0)
		{
			ErrorMessage = LOCTEXT("DropTableInvalidWeight", "Weight must be larger than 0!");
			Context.AddWarning(ErrorMessage);
			HasError = true;
		}
	}
	if (HasError) return EDataValidationResult::Invalid;
	return Super::IsDataValid(Context);
}

#undef LOCTEXT_NAMESPACE

void UItemSourcePool::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	CalculatePercentages();
}

void UItemSourcePool::PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent)
{
	Super::PostEditChangeChainProperty(PropertyChangedEvent);
	CalculatePercentages();
}

void UItemSourcePool::CalculatePercentages()
{
	/**
	 * Sum all weights into a total weight value, while also adjusting the weight of each drop to include to weight
	 * of all drops before it.
	 */

	int32 WeightSum = 0;
	for (FWeightedDrop& Entry : DropList)
	{
		WeightSum += Entry.Weight;
		Entry.AdjustedWeight = WeightSum;
	}

	for (FWeightedDrop& Entry : DropList)
	{
		Entry.AdjustedWeight /= WeightSum;
		Entry.PercentageChanceToDrop = 100.f * (static_cast<float>(Entry.Weight) / static_cast<float>(WeightSum));
	}
}

void UItemSourcePool::SortTable()
{
	Algo::SortBy(DropList, &FWeightedDrop::Weight);
}

#endif

FTableDrop UItemSourcePool::GenerateDrop_Internal(const double RanWeight) const
{
	if (DropList.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("Exiting generation: Empty Table"));
		return FTableDrop();
	}

	const int32 BinarySearchResult = Algo::LowerBoundBy(DropList, RanWeight, &FWeightedDrop::AdjustedWeight);

	if (!DropList.IsValidIndex(BinarySearchResult))
	{
		UE_LOG(LogTemp, Error, TEXT("Binary search returned out-of-bounds index!"));
		return FTableDrop();
	}

	return DropList[BinarySearchResult].Drop;
}

FFaerieAssetInfo UItemSourcePool::GetSourceInfo() const
{
	return TableInfo;
}

UFaerieItem* UItemSourcePool::CreateItemInstance(UObject* Outer) const
{
	return nullptr;
}

UFaerieItem* UItemSourcePool::CreateItemInstance(const UItemInstancingContext* Context) const
{
	const UItemInstancingContext_Crafting* CraftingContent = Cast<UItemInstancingContext_Crafting>(Context);

	if (!IsValid(CraftingContent))
	{
		return nullptr;
	}

	FTableDrop Drop;

	if (IsValid(CraftingContent->Squirrel))
	{
		Drop = GenerateDrop(CraftingContent->Squirrel);
	}
	else
	{
		Drop = GenerateDrop_NonSeeded();
	}

	if (Drop.IsValid())
	{
		return Drop.Resolve(CraftingContent);
	}

	return nullptr;
}

FTableDrop UItemSourcePool::GenerateDrop(USquirrel* Squirrel) const
{
	if (!ensure(IsValid(Squirrel)))
	{
		UE_LOG(LogTemp, Error, TEXT("Exiting generation: Invalid Squirrel"));
		return FTableDrop();
	}

	return GenerateDrop_Internal(Squirrel->NextReal());
}

FTableDrop UItemSourcePool::GenerateDrop_NonSeeded() const
{
	return GenerateDrop_Internal(FMath::FRand());
}