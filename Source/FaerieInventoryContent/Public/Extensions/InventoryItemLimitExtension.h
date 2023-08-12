// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "InventoryExtensionBase.h"
#include "InventoryItemLimitExtension.generated.h"

/**
 * An inventory extension that limits the number of entries or total item count that an inventory can hold.
 */
UCLASS()
class FAERIEINVENTORYCONTENT_API UInventoryItemLimitExtension : public UInventoryExtensionBase
{
	GENERATED_BODY()

protected:
	//~ UInventoryExtensionBase
	virtual void InitializeExtension(const UFaerieItemContainerBase* Container) override;
	virtual void DeinitializeExtension(const UFaerieItemContainerBase* Container) override;
	virtual EEventExtensionResponse AllowsAddition(const UFaerieItemContainerBase* Container, FFaerieItemStackView Stack) override;
	virtual void PostAddition(const UFaerieItemContainerBase* Container, const Faerie::FItemContainerEvent& Event) override;
	virtual void PostRemoval(const UFaerieItemContainerBase* Container, const Faerie::FItemContainerEvent& Event) override;
	virtual void PostEntryChanged(const UFaerieItemContainerBase* Container, const FEntryKey Key) override;
	//~ UInventoryExtensionBase

public:
	// Retrieve the number of items that this inventory contains.
	UFUNCTION(BlueprintPure, Category = "Inventory|Key")
	int32 GetTotalItemCount() const;

	// Retrieve the number of entries left to be filled.
	UFUNCTION(BlueprintPure, Category = "Inventory|Key")
	int32 GetRemainingStackCount() const;

	// Retrieve the number of items that this inventory can still contain.
	UFUNCTION(BlueprintPure, Category = "Inventory|Key")
	int32 GetRemainingTotalItemCount() const;

private:
	bool CanContain(FFaerieItemStackView Stack) const;

	void UpdateCacheForEntry(const UFaerieItemContainerBase* Container, FEntryKey Key);

protected:
	// Maximum number of entries the inventory can contain. 0 allows unlimited entries.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config", AdvancedDisplay, meta = (ClampMin = 0))
	int32 MaxEntries = 0;

	// Maximum number of stacks the inventory can contain. 0 allows unlimited stacks.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config", AdvancedDisplay, meta = (ClampMin = 0))
	int32 MaxStacks = 0;

	// Maximum number of items across all stacks the inventory can contain. 0 allows unlimited items.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config", AdvancedDisplay, meta = (ClampMin = 0))
	int32 MaxItems = 0;

private:
	UPROPERTY()
	TMap<FEntryKey, int32> StackAmountCache;

	UPROPERTY()
	int32 CurrentTotalItemCount = 0;
};
