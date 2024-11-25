// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "Extensions/InventorySimpleGridExtension.h"

#include "FaerieItemContainerBase.h"
#include "FaerieItemStorage.h"
#include "ItemContainerEvent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(InventorySimpleGridExtension)

void UInventorySimpleGridExtension::InitializeExtension(const UFaerieItemContainerBase* Container)
{
	checkf(!IsValid(InitializedContainer), TEXT("InventorySimpleGridExtension doesn't support multi-initialization!"))
	InitializedContainer = const_cast<UFaerieItemContainerBase*>(Container);
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, InitializedContainer, this);

	// Add all existing items to the grid on startup.
	// This is dumb, and just adds them in order, it doesn't space pack them. To do that, we would want to sort items by size, and add largest first.
	// This is also skipping possible serialization of grid data.
	// @todo handle serialization loading
	// @todo handle items that are too large to fit / too many items (log error?)
	OccupiedCells.SetNum(GridSize.X * GridSize.Y, false);
	if (const UFaerieItemStorage* ItemStorage = Cast<UFaerieItemStorage>(Container))
	{
		bool Failed = false;

		ItemStorage->ForEachKey(
			[this, ItemStorage, &Failed](const FEntryKey Key)
			{
				// @todo there is no break logic for ForEachKey! this is a temp hack
				if (Failed) return;

				for (const auto EntryView = ItemStorage->GetEntryView(Key);
					auto&& Entry : EntryView.Get().Stacks)
				{
					if (const FInventoryKey InvKey(Key, Entry.Key);
						!AddItemToGrid(InvKey, EntryView.Get().ItemObject))
					{
						Failed = true;
						break;
					}
				}
			});
	}
}

EEventExtensionResponse UInventorySimpleGridExtension::AllowsAddition(const UFaerieItemContainerBase* Container,
																	   const FFaerieItemStackView Stack,
																	   EFaerieStorageAddStackBehavior)
{
	// @todo add boolean in config to allow items without a shape
	if (!CanAddItemToGrid())
	{
		return EEventExtensionResponse::Disallowed;
	}

	return EEventExtensionResponse::Allowed;
}

void UInventorySimpleGridExtension::PostAddition(const UFaerieItemContainerBase* Container,
												  const Faerie::Inventory::FEventLog& Event)
{
	// @todo don't add items for existing keys

	FInventoryKey NewKey;
	NewKey.EntryKey = Event.EntryTouched;

	for (const FStackKey& StackKey : Event.StackKeys)
	{
		NewKey.StackKey = StackKey;
		AddItemToGrid(NewKey, Event.Item.Get());
	}
}

void UInventorySimpleGridExtension::PostRemoval(const UFaerieItemContainerBase* Container,
												 const Faerie::Inventory::FEventLog& Event)
{
	if (const UFaerieItemStorage* ItemStorage = Cast<UFaerieItemStorage>(Container))
	{
		for (auto&& StackKey : Event.StackKeys)
		{
			if (FInventoryKey CurrentKey{Event.EntryTouched, StackKey};
				ItemStorage->IsValidKey(CurrentKey))
			{
				PostStackChange({ CurrentKey, GetStackPlacementData(CurrentKey) });
			}
		}
	}
}

void UInventorySimpleGridExtension::PostEntryChanged(const UFaerieItemContainerBase* Container, FEntryKey Key)
{
	// Create a temporary array to store keys that need to be removed
	TArray<FInventoryKey> KeysToRemove;

	// get keys to remove
	for (const auto& SpatialEntry : GridContent)
	{
		if (const UFaerieItemStorage* Storage = Cast<UFaerieItemStorage>(InitializedContainer);
			!Storage->IsValidKey(SpatialEntry.Key))
		{
			KeysToRemove.Add(SpatialEntry.Key);
		}
		else
		{
			SpatialStackChangedNative.Broadcast(SpatialEntry.Key, EFaerieGridEventType::ItemChanged);
			SpatialStackChangedDelegate.Broadcast(SpatialEntry.Key, EFaerieGridEventType::ItemChanged);
		}
	}

	// remove the stored keys
	for (const FInventoryKey& KeyToRemove : KeysToRemove)
	{
		GridContent.Remove(KeyToRemove);
		SpatialStackChangedNative.Broadcast(KeyToRemove, EFaerieGridEventType::ItemRemoved);
		SpatialStackChangedDelegate.Broadcast(KeyToRemove, EFaerieGridEventType::ItemRemoved);
	}
	GridContent.MarkArrayDirty();
}


void UInventorySimpleGridExtension::PreStackRemove(const FFaerieGridKeyedStack& Stack)
{
	// This is to account for removals through proxies that don't directly interface with the grid
	OccupiedCells[Ravel(Stack.Value.Origin)] = false;

	SpatialStackChangedNative.Broadcast(Stack.Key, EFaerieGridEventType::ItemRemoved);
	SpatialStackChangedDelegate.Broadcast(Stack.Key, EFaerieGridEventType::ItemRemoved);
}

void UInventorySimpleGridExtension::PostStackAdd(const FFaerieGridKeyedStack& Stack)
{
	SpatialStackChangedNative.Broadcast(Stack.Key, EFaerieGridEventType::ItemAdded);
	SpatialStackChangedDelegate.Broadcast(Stack.Key, EFaerieGridEventType::ItemAdded);
}

void UInventorySimpleGridExtension::PostStackChange(const FFaerieGridKeyedStack& Stack)
{
	if (const UFaerieItemStorage* Storage = Cast<UFaerieItemStorage>(InitializedContainer); Storage->IsValidKey(Stack.Key))
	{
		SpatialStackChangedNative.Broadcast(Stack.Key, EFaerieGridEventType::ItemChanged);
		SpatialStackChangedDelegate.Broadcast(Stack.Key, EFaerieGridEventType::ItemChanged);
	}
}

bool UInventorySimpleGridExtension::CanAddItemToGrid() const
{
	const FFaerieGridPlacement TestPlacement = FindFirstEmptyLocation();
	return TestPlacement.Origin != FIntPoint::NoneValue;
}

bool UInventorySimpleGridExtension::AddItemToGrid(const FInventoryKey& Key, const UFaerieItem* Item)
{
	if (!Key.IsValid())
	{
		return false;
	}

	if (GridContent.Find(Key) != nullptr)
	{
		return true;
	}

	const FFaerieGridPlacement DesiredItemPlacement = FindFirstEmptyLocation();

	if (DesiredItemPlacement.Origin == FIntPoint::NoneValue)
	{
		return false;
	}

	GridContent.Insert(Key, DesiredItemPlacement);
	OccupiedCells[Ravel(DesiredItemPlacement.Origin)] = true;
	return true;
}

void UInventorySimpleGridExtension::RemoveItem(const FInventoryKey& Key, const UFaerieItem* Item)
{
	GridContent.Remove(Key);
}

void UInventorySimpleGridExtension::RemoveItemBatch(const TConstArrayView<FInventoryKey>& Keys, const UFaerieItem* Item)
{
	for (auto&& InvKey : Keys)
	{
		GridContent.Remove(InvKey);
	}
}

FFaerieGridPlacement UInventorySimpleGridExtension::FindFirstEmptyLocation() const
{
	// Early exit if grid is empty or invalid
	if (GridSize.X <= 0 || GridSize.Y <= 0)
	{
		return FFaerieGridPlacement{FIntPoint::NoneValue};
	}

	// For each cell in the grid
	FIntPoint TestPoint = FIntPoint::ZeroValue;
	for (TestPoint.Y = 0; TestPoint.Y < GridSize.Y; TestPoint.Y++)
	{
		for (TestPoint.X = 0; TestPoint.X < GridSize.X; TestPoint.X++)
		{
			// Skip if current cell is occupied
			if (OccupiedCells[Ravel(TestPoint)])
			{
				continue;
			}

			return FFaerieGridPlacement(TestPoint);
		}
	}
	// No valid placement found
	return FFaerieGridPlacement{FIntPoint::NoneValue};
}

bool UInventorySimpleGridExtension::MoveItem(const FInventoryKey& Key, const FIntPoint& TargetPoint)
{
	return GridContent.EditItem(Key,
		[&](FFaerieGridPlacement& Placement)
		{
			if (const FInventoryKey OverlappingKey = FindOverlappingItem(Key);
				OverlappingKey.IsValid())
			{
				// Gross method of getting mutable access to the other point... considering implementing handles like FInventoryContent does.
				FFaerieGridPlacement* OverlappingPlacement = nullptr;
				GridContent.EditItem(OverlappingKey,
					[&OverlappingPlacement](FFaerieGridPlacement& OtherPlacement)
					{
						OverlappingPlacement = &OtherPlacement;
						return true;
					});
				check(OverlappingPlacement);

				// If the Entry keys are identical, it gives us some other things to test before Swapping.
				if (Key.EntryKey == OverlappingKey.EntryKey)
				{
					if (Key.StackKey == OverlappingKey.StackKey)
					{
						// It's the same stack? No point in this!
						return false;
					}

					// Try merging them. This is known to be safe, since all stacks with the same key share immutability.
					if (UFaerieItemStorage* Storage = Cast<UFaerieItemStorage>(InitializedContainer);
						Storage->MergeStacks(Key.EntryKey, Key.StackKey, OverlappingKey.StackKey))
					{
						return true;
					}
				}

				SwapItems(Placement, *OverlappingPlacement);
				return true;
			}

			MoveSingleItem(Placement, TargetPoint);
			return true;
		});
}

FInventoryKey UInventorySimpleGridExtension::FindOverlappingItem(const FInventoryKey& ExcludeKey) const
{
	if (GridContent.Contains(ExcludeKey))
	{
		return GridContent.GetElement(ExcludeKey).Key;
	}
	return FInventoryKey();
}

void UInventorySimpleGridExtension::SwapItems(FFaerieGridPlacement& PlacementA, FFaerieGridPlacement& PlacementB)
{
	Swap(PlacementA.Origin, PlacementB.Origin);
}

void UInventorySimpleGridExtension::MoveSingleItem(FFaerieGridPlacement& Placement, const FIntPoint& NewPosition)
{
	UpdateItemPosition(Placement, NewPosition);
}

void UInventorySimpleGridExtension::UpdateItemPosition(FFaerieGridPlacement& Placement, const FIntPoint& NewPosition)
{
	// Clear old position first
	OccupiedCells[Ravel(Placement.Origin)] = false;

	// Then set new positions
	OccupiedCells[Ravel(NewPosition)] = true;

	Placement.Origin = NewPosition;
}

bool UInventorySimpleGridExtension::RotateItem(const FInventoryKey& Key)
{
	return GridContent.EditItem(Key,
		[this](FFaerieGridPlacement& Placement)
		{
			Placement.Rotation = GetNextRotation(Placement.Rotation);
			return true;
		});
}