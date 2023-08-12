// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "FaerieInfoObject.h"
#include "UObject/Object.h"
#include "FaerieItemSource.h"
#include "GenerationStructsLibrary.h"

#include "ItemSourcePool.generated.h"

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
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent) override;

	// Calculate the percentage each drop has to be chosen.
	void CalculatePercentages();

	// Keeps the table sorted by Weight.
	void SortTable();

	// Editor accessors
	static FName GetMemberName_DropList() { return GET_MEMBER_NAME_CHECKED(ThisClass, DropList); }
#endif

protected:
	FTableDrop GenerateDrop_Internal(double RanWeight) const;

public:
	//~ IFaerieItemSource
	virtual bool CanBeMutable() const override { return HasMutableDrops; }
	virtual FFaerieAssetInfo GetSourceInfo() const override;
	virtual UFaerieItem* CreateItemInstance(UObject* Outer) const override;
	virtual UFaerieItem* CreateItemInstance(const UItemInstancingContext* Context) const override;
	//~ IFaerieItemSource

	// Generates a drop from this table using a Squirrel as a RNG provider.
	UFUNCTION(Blueprintable, BlueprintPure = false, Category = "Faerie|ItemSourcePool", meta = (DisplayName = "Generate Drop (Seeded)"))
	FTableDrop GenerateDrop(USquirrel* Squirrel) const;

	// Generates a drop from this table using FRand as a RNG provider.
	UFUNCTION(Blueprintable, BlueprintPure = false, Category = "Faerie|ItemSourcePool", meta = (DisplayName = "Generate Drop (Non-Seeded)"))
	FTableDrop GenerateDrop_NonSeeded() const;

protected:
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Table")
	FFaerieAssetInfo TableInfo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Table")
	TArray<FWeightedDrop> DropList;

private:
	UPROPERTY()
	bool HasMutableDrops;
};