// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "FaerieGridStructs.h"
#include "InventoryGridExtensionBase.h"
#include "InventorySimpleGridExtension.generated.h"

/**
 *
 */
UCLASS()
class FAERIEINVENTORYCONTENT_API UInventorySimpleGridExtension : public UInventoryGridExtensionBase
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

public:
	bool CanAddItemToGrid() const;

	bool MoveItem(const FInventoryKey& Key, const FIntPoint& TargetPoint);
	bool RotateItem(const FInventoryKey& Key);

	FFaerieGridPlacement FindFirstEmptyLocation() const;

protected:
	FInventoryKey FindOverlappingItem(const FInventoryKey& ExcludeKey) const;

	void SwapItems(FFaerieGridPlacement& PlacementA, FFaerieGridPlacement& PlacementB);
	void MoveSingleItem(FFaerieGridPlacement& Placement, const FIntPoint& NewPosition);
};