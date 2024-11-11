// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "FaerieAssetInfo.h"
#include "UObject/Object.h"
#include "FaerieItemSource.h"
#include "GenerationStructs.h"

#include "ItemSourcePool.generated.h"

USTRUCT()
struct FFaerieWeightedDropPool
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Table")
	TArray<FWeightedDrop> DropList;

	// Generates a drop from this pool, using the provided random weight, which must be a value between 0 and 1.
	FTableDrop GenerateDrop(double RanWeight) const;

#if WITH_EDITOR
	// Calculate the percentage each drop has to be chosen.
	void CalculatePercentages();

	// Keeps the table sorted by Weight.
	void SortTable();
#endif
};

class USquirrel;

/**
 * An ItemSourcePool is a list of possible item generations, each with a unique weight.
 */
UCLASS()
class FAERIEITEMGENERATOR_API UItemSourcePool : public UObject, public IFaerieItemSource
{
	GENERATED_BODY()

public:
	UItemSourcePool();

	virtual void PreSave(FObjectPreSaveContext SaveContext) override;
	virtual void PostLoad() override;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent) override;
#endif

public:
	//~ IFaerieItemSource
	virtual bool CanBeMutable() const override { return HasMutableDrops; }
	virtual FFaerieAssetInfo GetSourceInfo() const override;
	virtual UFaerieItem* CreateItemInstance(UObject* Outer) const override;
	virtual UFaerieItem* CreateItemInstance(const UItemInstancingContext* Context) const override;
	//~ IFaerieItemSource

	// Generates a drop from this table, using the provided random weight, which must be a value between 0 and 1.
	UFUNCTION(Blueprintable, BlueprintPure = false, Category = "Faerie|ItemSourcePool")
	FTableDrop GenerateDrop(double RanWeight) const;

	UFUNCTION(Blueprintable, BlueprintPure = false, Category = "Faerie|ItemSourcePool", DisplayName = "Generate Drop (Seeded)")
	FTableDrop GenerateDrop_Seeded(USquirrel* Squirrel) const;

protected:
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Table", meta = (Pr))
	FFaerieAssetInfo TableInfo;

	UPROPERTY(EditAnywhere, Category = "Table")
	FFaerieWeightedDropPool DropPool;

private:
	UPROPERTY()
	bool HasMutableDrops = false;
};