// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "Extensions/InventorySpatialGridExtension.h"

#include "FaerieItemContainerBase.h"
#include "FaerieItemStorage.h"
#include "ItemContainerEvent.h"
#include "Tokens/FaerieShapeToken.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(InventorySpatialGridExtension)

DECLARE_STATS_GROUP(TEXT("InventorySpatialGridExtension"), STATGROUP_FaerieSpatialGrid, STATCAT_Advanced);
DECLARE_CYCLE_STAT(TEXT("Client OccupiedCells rebuild"), STAT_Client_CellRebuild, STATGROUP_FaerieSpatialGrid);

void UInventorySpatialGridExtension::InitializeExtension(const UFaerieItemContainerBase* Container)
{
	Super::InitializeExtension(Container);

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

EEventExtensionResponse UInventorySpatialGridExtension::AllowsAddition(const UFaerieItemContainerBase* Container,
																	   const FFaerieItemStackView Stack,
																	   EFaerieStorageAddStackBehavior) const
{
	// @todo add boolean in config to allow items without a shape
	if (!CanAddItemToGrid(GetItemShape_Impl(Stack.Item.Get())))
	{
		return EEventExtensionResponse::Disallowed;
	}

	return EEventExtensionResponse::Allowed;
}

void UInventorySpatialGridExtension::PostAddition(const UFaerieItemContainerBase* Container,
												const Faerie::Inventory::FEventLog& Event)
{
	// @todo don't add items for existing keys

	FInventoryKey NewKey;
	NewKey.EntryKey = Event.EntryTouched;

	for (const FStackKey StackKey : Event.StackKeys)
	{
		NewKey.StackKey = StackKey;
		AddItemToGrid(NewKey, Event.Item.Get());
	}
}

void UInventorySpatialGridExtension::PostRemoval(const UFaerieItemContainerBase* Container,
												 const Faerie::Inventory::FEventLog& Event)
{
	if (const UFaerieItemStorage* ItemStorage = Cast<UFaerieItemStorage>(Container))
	{
		// Create a temporary array to store keys that need to be removed
		TArray<FInventoryKey> KeysToRemove;

		for (auto&& StackKey : Event.StackKeys)
		{
			if (FInventoryKey CurrentKey{Event.EntryTouched, StackKey};
				ItemStorage->IsValidKey(CurrentKey))
			{
				PostStackChange({ CurrentKey, GetStackPlacementData(CurrentKey) });
			}
			else
			{
				KeysToRemove.Add(CurrentKey);
			}
		}
		RemoveItemBatch(KeysToRemove, Event.Item.Get());
	}
}

EEventExtensionResponse UInventorySpatialGridExtension::AllowsEdit(const UFaerieItemContainerBase* Container,
																	const FEntryKey Key, const FFaerieInventoryTag EditType) const
{
	if (EditType == Faerie::Inventory::Tags::Split)
	{
		if (!CanAddItemToGrid(GetItemShape_Impl(Container->View(Key).Item.Get())))
		{
			return EEventExtensionResponse::Disallowed;
		}
	}

	return EEventExtensionResponse::NoExplicitResponse;
}

void UInventorySpatialGridExtension::PostEntryChanged(const UFaerieItemContainerBase* Container, const Faerie::Inventory::FEventLog& Event)
{
	// Create a temporary array to store keys that need to be removed
	TArray<FInventoryKey> KeysToRemove;

	// get keys to remove
	for (auto&& AffectedKey : Event.StackKeys)
	{
		const FInventoryKey CurrentKey(Event.EntryTouched, AffectedKey);
		if (const UFaerieItemStorage* Storage = Cast<UFaerieItemStorage>(InitializedContainer);
			!Storage->IsValidKey(CurrentKey))
		{
			KeysToRemove.Add(CurrentKey);
		}
		else
		{
			if (GridContent.Find(CurrentKey) != nullptr)
			{
				BroadcastEvent(CurrentKey, EFaerieGridEventType::ItemChanged);
			}
			else
			{
				AddItemToGrid(CurrentKey, Event.Item.Get());
			}
		}
	}

	// remove the stored keys
	RemoveItemBatch(KeysToRemove, Event.Item.Get());
}

void UInventorySpatialGridExtension::PreStackRemove_Client(const FFaerieGridKeyedStack& Stack)
{
	RebuildOccupiedCells();

	BroadcastEvent(Stack.Key, EFaerieGridEventType::ItemRemoved);
}

void UInventorySpatialGridExtension::PreStackRemove_Server(const FFaerieGridKeyedStack& Stack, const UFaerieItem* Item)
{
	// This is to account for removals through proxies that don't directly interface with the grid
	const FFaerieGridShape Translated = ApplyPlacement(GetItemShape_Impl(Item), Stack.Value);
	RemoveItemPosition(Translated);

	BroadcastEvent(Stack.Key, EFaerieGridEventType::ItemRemoved);
}

void UInventorySpatialGridExtension::PostStackAdd(const FFaerieGridKeyedStack& Stack)
{
	BroadcastEvent(Stack.Key, EFaerieGridEventType::ItemAdded);
}

void UInventorySpatialGridExtension::PostStackChange(const FFaerieGridKeyedStack& Stack)
{
	if (const UFaerieItemStorage* Storage = Cast<UFaerieItemStorage>(InitializedContainer);
		Storage->IsValidKey(Stack.Key))
	{
		BroadcastEvent(Stack.Key, EFaerieGridEventType::ItemChanged);
	}
}

bool UInventorySpatialGridExtension::AddItemToGrid(const FInventoryKey& Key, const UFaerieItem* Item)
{
	if (!Key.IsValid())
	{
		return false;
	}

	if (GridContent.Find(Key) != nullptr)
	{
		return true;
	}

	FFaerieGridShape Shape = GetItemShape_Impl(Item);

	const FFaerieGridPlacement DesiredItemPlacement = FindFirstEmptyLocation(Shape);

	if (DesiredItemPlacement.Origin == FIntPoint::NoneValue)
	{
		return false;
	}

	GridContent.Insert(Key, DesiredItemPlacement);

	ApplyPlacementInline(Shape, DesiredItemPlacement);
	AddItemPosition(Shape);

	return true;
}

void UInventorySpatialGridExtension::RemoveItem(const FInventoryKey& Key, const UFaerieItem* Item)
{
	GridContent.BSOA::Remove(Key,
		[Item, this](const FFaerieGridKeyedStack& Stack)
		{
			PreStackRemove_Server(Stack, Item);
		});
}

void UInventorySpatialGridExtension::RemoveItemBatch(const TConstArrayView<FInventoryKey>& Keys, const UFaerieItem* Item)
{
	for (const FInventoryKey& KeyToRemove : Keys)
	{
		RemoveItem(KeyToRemove, Item);
		BroadcastEvent(KeyToRemove, EFaerieGridEventType::ItemRemoved);
	}
	GridContent.MarkArrayDirty();
}

void UInventorySpatialGridExtension::RebuildOccupiedCells()
{
	SCOPE_CYCLE_COUNTER(STAT_Client_CellRebuild);

	UnmarkAllCells();

	for (const auto& SpatialEntry : GridContent)
	{
		if (!InitializedContainer->IsValidKey(SpatialEntry.Key.EntryKey))
		{
			continue;
		}

		if (auto&& Item = InitializedContainer->View(SpatialEntry.Key.EntryKey).Item.Get())
		{
			const FFaerieGridShape Translated = ApplyPlacement(GetItemShape_Impl(Item), SpatialEntry.Value);
			AddItemPosition(Translated);
		}
	}
}

FFaerieGridShape UInventorySpatialGridExtension::GetItemShape_Impl(const UFaerieItem* Item) const
{
	if (IsValid(Item))
	{
		if (const UFaerieShapeToken* ShapeToken = Item->GetToken<UFaerieShapeToken>())
		{
			return ShapeToken->GetShape();
		}
		return FFaerieGridShape::MakeSquare(1);
	}
	return FFaerieGridShape();
}

bool UInventorySpatialGridExtension::CanAddItemToGrid(const FFaerieGridShapeConstView& Shape) const
{
	const FFaerieGridPlacement TestPlacement = FindFirstEmptyLocation(Shape);
	return TestPlacement.Origin != FIntPoint::NoneValue;
}

bool UInventorySpatialGridExtension::MoveItem(const FInventoryKey& Key, const FIntPoint& TargetPoint)
{
	const FFaerieGridShape ItemShape = GetItemShape(Key.EntryKey);

	// Create placement at target point
	FFaerieGridPlacement NewPlacement = GetStackPlacementData(Key);
	NewPlacement.Origin = TargetPoint;
	// Get the rotated shape based on current stack rotation so we can correctly get items that would overlap
	const FFaerieGridShape NewShape = ApplyPlacement(ItemShape, NewPlacement);

	// If this new position overlaps an existing item
	if (const FInventoryKey OverlappingKey = FindOverlappingItem(NewShape, Key);
		OverlappingKey.IsValid())
	{
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

		const FFaerieGridContent::FScopedStackHandle StackHandleA = GridContent.GetHandle(Key);
		const FFaerieGridContent::FScopedStackHandle StackHandleB = GridContent.GetHandle(OverlappingKey);

		return TrySwapItems(
			Key, StackHandleA.Get(),
			OverlappingKey, StackHandleB.Get());
	}

	// Copied logic from MoveSingleItem, but optimized to use existing variables.
	{
		if (!FitsInGrid(NewShape, MakeArrayView(&Key, 1)))
		{
			return false;
		}

		const FFaerieGridContent::FScopedStackHandle StackHandle = GridContent.GetHandle(Key);

		const FFaerieGridShape OldShape = ApplyPlacement(ItemShape, StackHandle.Get());
		RemoveItemPosition(OldShape);
		StackHandle->Origin = TargetPoint;
		AddItemPosition(NewShape);
	}

	return true;
}

bool UInventorySpatialGridExtension::RotateItem(const FInventoryKey& Key)
{
	FFaerieGridShape ItemShape = GetItemShape(Key.EntryKey);

	// No Point in Trying to Rotate
	if (ItemShape.IsSymmetrical()) return false;

	FFaerieGridPlacement NewPlacement = GetStackPlacementData(Key);
	NewPlacement.Rotation = GetNextRotation(NewPlacement.Rotation);

	const FFaerieGridShape NewShape = ApplyPlacement(ItemShape, NewPlacement);
	if (!FitsInGrid(NewShape, MakeArrayView(&Key, 1)))
	{
		return false;
	}

	const FFaerieGridContent::FScopedStackHandle Handle = GridContent.GetHandle(Key);

	// Store old points before transformations so we can clear them from the bit grid
	ApplyPlacementInline(ItemShape, Handle.Get());

	// Clear old occupied cells
	RemoveItemPosition(ItemShape);

	Handle->Rotation = NewPlacement.Rotation;

	// Set new occupied cells taking into account rotation
	AddItemPosition(NewShape);

	return true;
}

FFaerieGridShape UInventorySpatialGridExtension::GetItemShape(const FEntryKey Key) const
{
	if (IsValid(InitializedContainer))
	{
		const FFaerieItemStackView View = InitializedContainer->View(Key);
		return GetItemShape_Impl(View.Item.Get());
	}

	return FFaerieGridShape();
}

FIntPoint UInventorySpatialGridExtension::GetStackBounds(const FInventoryKey& Key) const
{
	return GetItemShape(Key.EntryKey).GetSize();
}

FFaerieGridShape UInventorySpatialGridExtension::ApplyPlacement(const FFaerieGridShapeConstView& Shape, const FFaerieGridPlacement& Placement)
{
	return Shape.Copy().Rotate(Placement.Rotation).Translate(Placement.Origin);
}

void UInventorySpatialGridExtension::ApplyPlacementInline(FFaerieGridShape& Shape, const FFaerieGridPlacement& Placement)
{
	Shape.RotateInline(Placement.Rotation);
	Shape.TranslateInline(Placement.Origin);
}

FFaerieGridPlacement UInventorySpatialGridExtension::FindFirstEmptyLocation(const FFaerieGridShapeConstView& Shape) const
{
	// Early exit if grid is empty or invalid
	if (GridSize.X <= 0 || GridSize.Y <= 0)
	{
		return FFaerieGridPlacement{FIntPoint::NoneValue};
	}

	// Determine which rotations to check
	TArray<ESpatialItemRotation> RotationRange;
	if (Shape.IsSymmetrical())
	{
		RotationRange.Add(ESpatialItemRotation::None);
	}
	else
	{
		for (const ESpatialItemRotation Rotation : TEnumRange<ESpatialItemRotation>())
		{
			RotationRange.Add(Rotation);
		}
	}

	// Find top left most point
	FIntPoint FirstPoint = FIntPoint(TNumericLimits<int32>::Max());
	for (const FIntPoint& Point : Shape.Points)
	{
		if (Point.Y < FirstPoint.Y || (Point.Y == FirstPoint.Y && Point.X < FirstPoint.X))
		{
			FirstPoint = Point;
		}
	}

	FFaerieGridPlacement TestPlacement;

	// For each cell in the grid
	FIntPoint TestPoint = FIntPoint::ZeroValue;
	for (TestPoint.Y = 0; TestPoint.Y < GridSize.Y; TestPoint.Y++)
	{
		for (TestPoint.X = 0; TestPoint.X < GridSize.X; TestPoint.X++)
		{
			// Skip if current cell is occupied
			if (IsCellOccupied(TestPoint))
			{
				continue;
			}

			// Calculate the origin offset by the first point
			TestPlacement.Origin = TestPoint - FirstPoint;

			for (const ESpatialItemRotation Rotation : RotationRange)
			{
				TestPlacement.Rotation = Rotation;
				const FFaerieGridShape Translated = ApplyPlacement(Shape, TestPlacement); // @todo this is *way* too many array allocations. optimize this!
				if (FitsInGrid(Translated, {}))
				{
					return TestPlacement;
				}
			}
		}
	}
	// No valid placement found
	return FFaerieGridPlacement{FIntPoint::NoneValue};
}

bool UInventorySpatialGridExtension::FitsInGrid(const FFaerieGridShapeConstView& TranslatedShape, const TConstArrayView<FInventoryKey> ExcludedKeys) const
{
	// Calculate shape bounds
	const FInt32Rect Bounds = TranslatedShape.GetBounds();

	// Early exit if shape is obviously too large
	if (Bounds.Max.X > GridSize.X || Bounds.Max.Y > GridSize.Y)
	{
		return false;
	}

	// Build list of excluded indices
	TArray<FIntPoint> ExcludedPositions;
	ExcludedPositions.Reserve(ExcludedKeys.Num() * TranslatedShape.Points.Num());
	for (const FInventoryKey& Key : ExcludedKeys)
	{
		FFaerieGridShape OtherShape = GetItemShape(Key.EntryKey);
		ApplyPlacementInline(OtherShape, GetStackPlacementData(Key));
		for (const auto& Point : OtherShape.Points)
		{
			ExcludedPositions.Add(Point);
		}
	}

	// Check if all points in the shape fit within the grid and don't overlap with occupied cells
	for (const FIntPoint& Point : TranslatedShape.Points)
	{
		// Check if point is within grid bounds
		if (Point.X < 0 || Point.X >= GridSize.X ||
			Point.Y < 0 || Point.Y >= GridSize.Y)
		{
			return false;
		}

		// If this index is not in the excluded list, check if it's occupied
		if (!ExcludedPositions.Contains(Point) && IsCellOccupied(Point))
		{
			return false;
		}
	}

	return true;
}

bool UInventorySpatialGridExtension::FitsInGridAnyRotation(const FFaerieGridShapeConstView& Shape, const FIntPoint Origin, const TConstArrayView<FInventoryKey> ExcludedKeys) const
{
	FFaerieGridShape Translated = Shape.Copy().Translate(Origin);

	// Try 4 times if it FitsInGrid, rotating by 90 degrees between each test
	for (int32 i = 0; i < 4; ++i)
	{
		if (FitsInGrid(Translated, ExcludedKeys))
		{
			return true;
		}
		Translated.RotateInline(ESpatialItemRotation::Ninety);
	}
	return false;
}

FInventoryKey UInventorySpatialGridExtension::FindOverlappingItem(const FFaerieGridShapeConstView& TranslatedShape,
																  const FInventoryKey& ExcludeKey) const
{
	if (const FFaerieGridKeyedStack* Stack = GridContent.FindByPredicate(
		[this, &TranslatedShape, ExcludeKey](const FFaerieGridKeyedStack& Other)
		{
			if (ExcludeKey == Other.Key) { return false; }

			// Create a rotated and translated version of the other item's shape
			FFaerieGridShape OtherItemShape = GetItemShape(Other.Key.EntryKey);
			ApplyPlacementInline(OtherItemShape, Other.Value);
			return TranslatedShape.Overlaps(OtherItemShape);
		}))
	{
		return Stack->Key;
	}
	return FInventoryKey();
}

bool UInventorySpatialGridExtension::TrySwapItems(const FInventoryKey KeyA, FFaerieGridPlacement& PlacementA,
												  const FInventoryKey KeyB, FFaerieGridPlacement& PlacementB)
{
	const FFaerieGridShape ItemShapeA = GetItemShape(KeyA.EntryKey);
	const FFaerieGridShape ItemShapeB = GetItemShape(KeyB.EntryKey);

	// Get new placements for both items
	FFaerieGridPlacement PlacementANew = PlacementA;
	FFaerieGridPlacement PlacementBNew = PlacementB;
	PlacementANew.Origin = PlacementB.Origin;
	PlacementBNew.Origin = PlacementA.Origin;

	// Check if both items can exist in their new positions without overlapping each other
	const FFaerieGridShape ItemShapeANew = ApplyPlacement(ItemShapeA, PlacementANew);
	const FFaerieGridShape ItemShapeBNew = ApplyPlacement(ItemShapeB, PlacementBNew);
	if (ItemShapeANew.Overlaps(ItemShapeANew))
	{
		return false;
	}

	// Check if both items fit inside the grid
	if (!FitsInGrid(ItemShapeANew, MakeArrayView(&KeyB, 1)) ||
		!FitsInGrid(ItemShapeBNew, MakeArrayView(&KeyA, 1)))
	{
		return false;
	}

	const FFaerieGridShape ItemShapeAOld = ApplyPlacement(ItemShapeA, PlacementA);
	const FFaerieGridShape ItemShapeBOld = ApplyPlacement(ItemShapeB, PlacementB);

	// Remove Old Positions
	RemoveItemPosition(ItemShapeAOld);
	RemoveItemPosition(ItemShapeBOld);
	// Add To Swapped Positions
	AddItemPosition(ItemShapeANew);
	AddItemPosition(ItemShapeBNew);
	Swap(PlacementA.Origin, PlacementB.Origin);

	return true;
}

bool UInventorySpatialGridExtension::MoveSingleItem(const FInventoryKey Key, FFaerieGridPlacement& Placement, const FIntPoint& NewPosition)
{
	FFaerieGridPlacement PlacementCopy = Placement;
	PlacementCopy.Origin = NewPosition;

	FFaerieGridShape ItemShape = GetItemShape(Key.EntryKey);
	const FFaerieGridShape NewShape = ApplyPlacement(ItemShape, PlacementCopy);

	if (!FitsInGrid(NewShape, MakeArrayView(&Key, 1)))
	{
		return false;
	}

	ApplyPlacementInline(ItemShape, Placement);

	RemoveItemPosition(ItemShape);
	Placement.Origin = NewPosition;
	AddItemPosition(NewShape);

	return true;
}

void UInventorySpatialGridExtension::AddItemPosition(const FFaerieGridShapeConstView TranslatedShape)
{
	for (auto& Point : TranslatedShape.Points)
	{
		MarkCell(Point);
	}
}

void UInventorySpatialGridExtension::RemoveItemPosition(const FFaerieGridShapeConstView& TranslatedShape)
{
	for (auto& Point : TranslatedShape.Points)
	{
		UnmarkCell(Point);
	}
}