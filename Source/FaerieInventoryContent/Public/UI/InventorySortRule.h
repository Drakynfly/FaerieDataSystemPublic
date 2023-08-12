// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "FaerieItemDataComparator.h"
#include "LocalInventoryEntryCache.h"
#include "InventorySortRule.generated.h"

/**
 * Sorting rule to organize a list of inventory entries in client UI. Will automatically fail if passed proxies that are
 * not InventoryEntryProxyBases.
 */
UCLASS(Abstract, Const, Blueprintable, BlueprintType, ClassGroup = "Inventory")
class FAERIEINVENTORYCONTENT_API UInventorySortRule : public UFaerieItemDataComparator
{
	GENERATED_BODY()

public:
	virtual bool Exec(const UFaerieItemDataProxyBase* A, const UFaerieItemDataProxyBase* B) const override final;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Sort Rule")
	bool Execute(const UInventoryEntryProxyBase* A, const UInventoryEntryProxyBase* B) const;

protected:
	// Text used to display to players.
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Sort Rule")
	FText RuleLabel;
};