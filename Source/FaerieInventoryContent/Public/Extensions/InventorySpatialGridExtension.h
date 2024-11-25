// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "FaerieGridStructs.h"
#include "InventoryGridExtensionBase.h"
#include "SpatialTypes.h"
#include "InventorySpatialGridExtension.generated.h"

/**
 *
 */
UCLASS()
class FAERIEINVENTORYCONTENT_API UInventorySpatialGridExtension : public UInventoryGridExtensionBase
{
	GENERATED_BODY()

protected:
	//~ UItemContainerExtensionBase
	virtual void InitializeExtension(const UFaerieItemContainerBase* Container) override;
	virtual EEventExtensionResponse AllowsAddition(const UFaerieItemContainerBase* Container, FFaerieItemStackView Stack, EFaerieStorageAddStackBehavior AddStackBehavior) override;
	virtual void PostAddition(const UFaerieItemContainerBase* Container, const Faerie::Inventory::FEventLog& Event) override;
	virtual void PostRemoval(const UFaerieItemContainerBase* Container, const Faerie::Inventory::FEventLog& Event) override;
	virtual void PostEntryChanged(const UFaerieItemContainerBase* Container, FEntryKey Key) override;
	//~ UItemContainerExtensionBase

	//~ UInventoryGridExtensionBase
	virtual void PreStackRemove(const FFaerieGridKeyedStack& Stack) override;
	virtual void PostStackAdd(const FFaerieGridKeyedStack& Stack) override;
	virtual void PostStackChange(const FFaerieGridKeyedStack& Stack) override;
	//~ UInventoryGridExtensionBase

private:
	bool AddItemToGrid(const FInventoryKey& Key, const UFaerieItem* Item);
	void RemoveItem(const FInventoryKey& Key, const UFaerieItem* Item);
	void RemoveItemBatch(const TConstArrayView<FInventoryKey>& Keys, const UFaerieItem* Item);

	// Gets a shape from a shape token on the item, or returns a single cell at 0,0 for items with no token.
	FFaerieGridShape GetItemShape_Impl(const UFaerieItem* Item) const;

public:
	bool CanAddItemToGrid(const FFaerieGridShapeConstView& Shape) const;

	bool MoveItem(const FInventoryKey& Key, const FIntPoint& TargetPoint);
	bool RotateItem(const FInventoryKey& Key);

	bool FitsInGrid(const FFaerieGridShapeConstView& Shape, const FFaerieGridPlacement& PlacementData, TConstArrayView<FInventoryKey> ExcludedKeys = {}, FIntPoint* OutCandidate = nullptr) const;

	bool FitsInGridAnyRotation(const FFaerieGridShapeConstView& Shape, FFaerieGridPlacement& PlacementData, TConstArrayView<FInventoryKey> ExcludedKeys = {}, FIntPoint* OutCandidate = nullptr) const;

	FFaerieGridPlacement FindFirstEmptyLocation(const FFaerieGridShapeConstView& Shape) const;

	FFaerieGridShape GetItemShape(FEntryKey Key) const;

	UFUNCTION(BlueprintCallable, Category = "Faerie|Grid")
	FIntPoint GetStackBounds(const FInventoryKey& Key) const;

protected:
	static FFaerieGridShape ApplyPlacement(const FFaerieGridShapeConstView& Shape, const FFaerieGridPlacement& Placement);

	FInventoryKey FindOverlappingItem(const FFaerieGridShapeConstView& TranslatedShape, const FInventoryKey& ExcludeKey) const;

	bool TrySwapItems(FInventoryKey KeyA, FFaerieGridPlacement& PlacementA, FInventoryKey KeyB, FFaerieGridPlacement& PlacementB);

	bool MoveSingleItem(const FInventoryKey Key, FFaerieGridPlacement& Placement, const FIntPoint& NewPosition);

	void UpdateItemPosition(const FInventoryKey Key, FFaerieGridPlacement& Placement, const FIntPoint& NewPosition);
};