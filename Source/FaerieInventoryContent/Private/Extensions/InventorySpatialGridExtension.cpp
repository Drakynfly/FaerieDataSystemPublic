// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "Extensions/InventorySpatialGridExtension.h"

#include "FaerieItemContainerBase.h"
#include "FaerieItemStorage.h"
#include "ItemContainerEvent.h"
#include "Net/UnrealNetwork.h"
#include "Tokens/FaerieShapeToken.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(InventorySpatialGridExtension)

void FSpatialKeyedEntry::PreReplicatedRemove(const FSpatialContent& InArraySerializer)
{
	InArraySerializer.PreEntryReplicatedRemove(*this);
}

void FSpatialKeyedEntry::PostReplicatedAdd(FSpatialContent& InArraySerializer)
{
	InArraySerializer.PostEntryReplicatedAdd(*this);
}

void FSpatialKeyedEntry::PostReplicatedChange(const FSpatialContent& InArraySerializer)
{
	InArraySerializer.PostEntryReplicatedChange(*this);
}

bool FSpatialContent::EditItem(const FInventoryKey Key, const TFunctionRef<void(FSpatialItemPlacement&)>& Func)
{
	if (const int32 Index = IndexOf(Key);
		Index != INDEX_NONE)
	{
		FSpatialKeyedEntry& Entry = Items[Index];
		Func(Entry.Value);
		MarkItemDirty(Entry);
		PostEntryReplicatedChange(Entry);
		return true;
	}
	return false;
}

void FSpatialContent::PreEntryReplicatedRemove(const FSpatialKeyedEntry& Entry) const
{
	if (ChangeListener.IsValid())
	{
		ChangeListener->PreEntryReplicatedRemove(Entry);
	}
}

void FSpatialContent::PostEntryReplicatedAdd(const FSpatialKeyedEntry& Entry)
{
	if (ChangeListener.IsValid())
	{
		ChangeListener->PostEntryReplicatedAdd(Entry);
	}
}

void FSpatialContent::PostEntryReplicatedChange(const FSpatialKeyedEntry& Entry) const
{
	if (ChangeListener.IsValid())
	{
		ChangeListener->PostEntryReplicatedChange(Entry);
	}
}

void FSpatialContent::Insert(FInventoryKey Key, const FSpatialItemPlacement& Value)
{
	check(Key.IsValid())

	FSpatialKeyedEntry& NewEntry = BSOA::Insert({Key, Value});

	PostEntryReplicatedAdd(NewEntry);
	MarkItemDirty(NewEntry);
}

void FSpatialContent::Remove(const FInventoryKey Key)
{
	if (BSOA::Remove(Key,
			[this](const FSpatialKeyedEntry& Entry)
			{
				// Notify owning server of this removal.
				PreEntryReplicatedRemove(Entry);
			}))
	{
		// Notify clients of this removal.
		MarkArrayDirty();
	}
}

void UInventorySpatialGridExtension::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams SharedParams;
	SharedParams.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, SpatialEntries, SharedParams);
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, GridSize, SharedParams);
}

void UInventorySpatialGridExtension::PostInitProperties()
{
	Super::PostInitProperties();
	SpatialEntries.ChangeListener = this;
}

void UInventorySpatialGridExtension::InitializeExtension(const UFaerieItemContainerBase* Container)
{
	// Add all existing items to the grid on startup.
	// This is dumb, and just adds them in order, it doesn't space pack them. To do that, we would want to sort items by size, and add largest first.
	// This is also skipping possible serialization of grid data.
	// @todo handle serialization loading
	// @todo handle items that are too large to fix / too many items (log error?)
	OccupiedCells.SetNum(GridSize.X * GridSize.Y, false);
	if (const UFaerieItemStorage* ItemStorage = Cast<UFaerieItemStorage>(Container))
	{
		ItemStorage->ForEachKey(
			[this, ItemStorage](const FEntryKey Key)
			{
				const FFaerieItemStackView View = ItemStorage->View(Key);
				const UFaerieShapeToken* Token = View.Item->GetToken<UFaerieShapeToken>();

				const TArray<FInventoryKey> InvKeys = ItemStorage->GetInvKeysForEntry(Key);

				for (auto&& InvKey : InvKeys)
				{
					AddItemToGrid(InvKey, Token);
				}
			});
	}
}

void UInventorySpatialGridExtension::DeinitializeExtension(const UFaerieItemContainerBase* Container)
{
	// Remove all entries for this container on shutdown.
	Container->ForEachKey(
		[this](const FEntryKey Key)
		{
			RemoveItemsForEntry(Key);
		});
}

EEventExtensionResponse UInventorySpatialGridExtension::AllowsAddition(const UFaerieItemContainerBase* Container,
																		const FFaerieItemStackView Stack)
{
	// @todo add boolean in config to allow items without a shape
	if (!CanAddItemToGrid(Stack.Item->GetToken<UFaerieShapeToken>()))
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

	for (const FStackKey& StackKey : Event.StackKeys)
	{
		NewKey.StackKey = StackKey;
		if (const UFaerieShapeToken* ShapeToken = Event.Item->GetToken<UFaerieShapeToken>())
		{
			AddItemToGrid(NewKey, ShapeToken);
		}
		else
		{
			AddItemToGrid(NewKey, nullptr);
		}
	}
}

void UInventorySpatialGridExtension::PostRemoval(const UFaerieItemContainerBase* Container,
												const Faerie::Inventory::FEventLog& Event)
{
	if (const UFaerieItemStorage* ItemStorage = Cast<UFaerieItemStorage>(Container))
	{
		if (!ItemStorage->IsValidKey(Event.EntryTouched))
		{
			RemoveItemsForEntry(Event.EntryTouched);
		}
		else
		{
			FInventoryKey Key;
			Key.EntryKey = Event.EntryTouched;

			for (const FStackKey& StackKey : Event.StackKeys)
			{
				Key.StackKey = StackKey;
				if (!ItemStorage->IsValidKey(Key))
				{
					RemoveItem(Key);
				}
			}
		}
	}
}

void UInventorySpatialGridExtension::PreEntryReplicatedRemove(const FSpatialKeyedEntry& Entry)
{
	// This is to account for removals through proxies that don't directly interface with the grid
	const FFaerieGridShape Temp = Entry.Value.GetRotated();
	for (auto&& Point : Temp.Points)
	{
		const FIntPoint OldPoint = Entry.Value.Origin + Point;
		const int32 OldBitGridIndex = Ravel(OldPoint);
		OccupiedCells[OldBitGridIndex] = false;
	}
	SpatialEntryChangedDelegateNative.Broadcast(Entry.Key, ESpatialEventType::ItemRemoved);
	SpatialEntryChangedDelegate.Broadcast(Entry.Key, ESpatialEventType::ItemRemoved);
}

void UInventorySpatialGridExtension::PostEntryReplicatedAdd(const FSpatialKeyedEntry& Entry)
{
	SpatialEntryChangedDelegateNative.Broadcast(Entry.Key, ESpatialEventType::ItemAdded);
	SpatialEntryChangedDelegate.Broadcast(Entry.Key, ESpatialEventType::ItemAdded);
}

void UInventorySpatialGridExtension::PostEntryReplicatedChange(const FSpatialKeyedEntry& Entry)
{
	SpatialEntryChangedDelegateNative.Broadcast(Entry.Key, ESpatialEventType::ItemChanged);
	SpatialEntryChangedDelegate.Broadcast(Entry.Key, ESpatialEventType::ItemChanged);
}

void UInventorySpatialGridExtension::OnRep_GridSize()
{
	GridSizeChangedDelegateNative.Broadcast(GridSize);
	GridSizeChangedDelegate.Broadcast(GridSize);
}

bool UInventorySpatialGridExtension::CanAddItemToGrid(const UFaerieShapeToken* ShapeToken) const
{
	FSpatialItemPlacement TestPlacement(ShapeToken->GetShape());
	FindFirstEmptyLocation(TestPlacement);
	return TestPlacement.Origin != FIntPoint::NoneValue;
}

bool UInventorySpatialGridExtension::AddItemToGrid(const FInventoryKey& Key, const UFaerieShapeToken* ShapeToken)
{
	if (!Key.IsValid())
	{
		return false;
	}

	if (SpatialEntries.Find(Key) != nullptr)
	{
		return true;
	}

	const FFaerieGridShape Shape = ShapeToken ? ShapeToken->GetShape() : FFaerieGridShape::MakeSquare(1);

	FSpatialItemPlacement DesiredItemPlacement(Shape);
	FindFirstEmptyLocation(DesiredItemPlacement);

	if (DesiredItemPlacement.Origin == FIntPoint::NoneValue)
	{
		return false;
	}

	SpatialEntries.Insert(Key, DesiredItemPlacement);
	for (const FFaerieGridShape RotatedShape = DesiredItemPlacement.GetRotated();
		 const FIntPoint& Point : RotatedShape.Points)
	{
		const FIntPoint Translated = Point + DesiredItemPlacement.Origin;
		OccupiedCells[Ravel(Translated)] = true;
	}
	return true;
}

void UInventorySpatialGridExtension::RemoveItem(const FInventoryKey& Key)
{
	SpatialEntries.Remove(Key);
}

void UInventorySpatialGridExtension::RemoveItemsForEntry(const FEntryKey& Key)
{
	TArray<FInventoryKey> Keys;
	for (auto&& Element : SpatialEntries)
	{
		if (Element.Key.EntryKey == Key)
		{
			Keys.Add(Element.Key);
			for (const FFaerieGridShape TempShape = Element.Value.GetRotated();
				 const FIntPoint& Point : TempShape.Points)
			{
				const FIntPoint Translated = Point + Element.Value.Origin;
				OccupiedCells[Ravel(Translated)] = false;
			}
		}
	}

	for (auto&& InvKey : Keys)
	{
		RemoveItem(InvKey);
	}
}

FSpatialItemPlacement UInventorySpatialGridExtension::GetEntryPlacementData(const FInventoryKey& Key) const
{
	if (auto&& Placement = SpatialEntries.Find(Key))
	{
		return *Placement;
	}

	return FSpatialItemPlacement();
}

FIntPoint UInventorySpatialGridExtension::GetEntryBounds(const FInventoryKey& Entry) const
{
	return GetEntryPlacementData(Entry).ItemShape.GetSize();
}

bool UInventorySpatialGridExtension::FitsInGrid(const FSpatialItemPlacement& PlacementData, const TConstArrayView<FInventoryKey> ExcludedKeys, FIntPoint* OutCandidate) const
{
	// Build list of excluded indices
	TArray<int32> ExcludedIndices;
	ExcludedIndices.Reserve(ExcludedKeys.Num() * PlacementData.ItemShape.Points.Num());
	for (const FInventoryKey& Key : ExcludedKeys)
	{
		const FSpatialItemPlacement Entry = GetEntryPlacementData(Key);
		for (const FFaerieGridShape Shape = Entry.GetRotated();
			 const auto& Point : Shape.Points)
		{
			const FIntPoint AbsolutePosition = Entry.Origin + Point;
			ExcludedIndices.Add(Ravel(AbsolutePosition));
		}
	}

	// Check if all points in the shape fit within the grid and don't overlap with occupied cells
	for (const FFaerieGridShape TransformedShape = PlacementData.GetRotated();
		 const FIntPoint& Point : TransformedShape.Points)
	{
		const FIntPoint AbsolutePosition = PlacementData.Origin + Point;

		// Check if point is within grid bounds
		if (AbsolutePosition.X < 0 || AbsolutePosition.X >= GridSize.X ||
			AbsolutePosition.Y < 0 || AbsolutePosition.Y >= GridSize.Y)
		{
			return false;
		}

		// If this index is not in the excluded list, check if it's occupied
		if (const int32 BitGridIndex = Ravel(AbsolutePosition);
			!ExcludedIndices.Contains(BitGridIndex) && OccupiedCells[BitGridIndex])
		{
			if (OutCandidate)
			{
				// Skip past this occupied cell
				OutCandidate->X = AbsolutePosition.X;
				OutCandidate->Y = PlacementData.Origin.Y;
			}
			return false;
		}
	}

	return true;
}

void UInventorySpatialGridExtension::FindFirstEmptyLocation(FSpatialItemPlacement& OutPlacementData) const
{
	// Early exit if grid is empty or invalid
	if (GridSize.X <= 0 || GridSize.Y <= 0)
	{
		OutPlacementData.Origin = FIntPoint::NoneValue;
		return;
	}

	// Determine which rotations to check
	TArray<ESpatialItemRotation> RotationRange;
	if (OutPlacementData.ItemShape.IsSymmetrical())
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
	int32 Min = TNumericLimits<int32>::Max();
	int32 Max = TNumericLimits<int32>::Min();
	for (const FIntPoint& Point : OutPlacementData.ItemShape.Points)
	{
		Min = FMath::Min(Min, Point.X);
		Max = FMath::Max(Max, Point.X);
	}
	const int32 LookAhead = Max - Min + 1;

	// For each cell in the grid
	FIntPoint TestPoints = FIntPoint::ZeroValue;
	for (TestPoints.Y = 0; TestPoints.Y < GridSize.Y; TestPoints.Y++)
	{
		for (TestPoints.X = 0; TestPoints.X < GridSize.X; TestPoints.X++)
		{
			// Skip if current cell is occupied
			if (OccupiedCells[TestPoints.Y * GridSize.X + TestPoints.X])
			{
				continue;
			}
			// Try each rotation at this potential origin point
			OutPlacementData.Origin = FIntPoint(TestPoints.X, TestPoints.Y);
			for (const ESpatialItemRotation Rotation : RotationRange)
			{
				OutPlacementData.Rotation = Rotation;
				if (FitsInGrid(OutPlacementData, TArray<FInventoryKey>(), &TestPoints))
				{
					OutPlacementData.PivotPoint = OutPlacementData.ItemShape.GetShapeCenter() + OutPlacementData.Origin;
					return;
				}
			}
		}
	}
	// No valid placement found
	OutPlacementData.Origin = FIntPoint::NoneValue;
}

void UInventorySpatialGridExtension::SetGridSize(const FIntPoint NewGridSize)
{
	if (GridSize != NewGridSize)
	{
		const FIntPoint OldSize = GridSize;
		TBitArray<> OldOccupied = OccupiedCells;

		// Resize to new dimensions
		GridSize = NewGridSize;
		OccupiedCells.Init(false, GridSize.X * GridSize.Y);

		// Copy over existing data that's still in bounds
		for (int32 y = 0; y < FMath::Min(OldSize.Y, GridSize.Y); y++)
		{
			for (int32 x = 0; x < FMath::Min(OldSize.X, GridSize.X); x++)
			{
				const int32 OldIndex = x + y * OldSize.X;
				const int32 NewIndex = x + y * GridSize.X;
				OccupiedCells[NewIndex] = OldOccupied[OldIndex];
			}
		}

		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, GridSize, this);

		// OnReps must be called manually on the server in c++
		OnRep_GridSize();
	}
}

bool UInventorySpatialGridExtension::MoveItem(const FInventoryKey& Key, const FIntPoint& TargetPoint)
{
	FSpatialKeyedEntry* MatchingEntry = FindItemByKey(Key);
	if (!MatchingEntry)
	{
		return false;
	}

	// Get the rotated shape based on current entry rotation so we can correctly get items that would overlap
	const FFaerieGridShape RotatedShape = MatchingEntry->Value.GetRotated();

	if (FSpatialKeyedEntry* OverlappingItem =
		FindOverlappingItem(RotatedShape.Translate(MatchingEntry->Value.Origin), Key))
	{
		return TrySwapItems(*MatchingEntry, *OverlappingItem);
	}

	return MoveSingleItem(*MatchingEntry, TargetPoint);
}

int32 UInventorySpatialGridExtension::Ravel(const FIntPoint& Point) const
{
	return Point.Y * GridSize.X + Point.X;
}

FIntPoint UInventorySpatialGridExtension::Unravel(const int32 Index) const
{
	const int32 X = Index % GridSize.X;
	const int32 Y = Index / GridSize.X;
	return FIntPoint{ X, Y };
}

FSpatialKeyedEntry* UInventorySpatialGridExtension::FindItemByKey(const FInventoryKey& Key)
{
	if (const int32 Index = SpatialEntries.IndexOf(Key);
		Index != INDEX_NONE)
	{
		return &SpatialEntries.Items[Index];
	}
	return nullptr;
}

FSpatialKeyedEntry* UInventorySpatialGridExtension::FindOverlappingItem(const FFaerieGridShape& TranslatedShape,
																		const FInventoryKey& ExcludeKey)
{
	return SpatialEntries.Items.FindByPredicate(
		[this, &TranslatedShape, ExcludeKey](const FSpatialKeyedEntry& Other)
		{
			if (ExcludeKey == Other.Key)
			{
				return false;
			}

			// Create a rotated and translated version of the other item's shape
			const FFaerieGridShape OtherTranslatedShape = Other.Value.GetRotated().Translate(Other.Value.Origin);

			return TranslatedShape.Contains(OtherTranslatedShape);
		});
}

bool UInventorySpatialGridExtension::TrySwapItems(FSpatialKeyedEntry& MovingItemA, FSpatialKeyedEntry& MovingItemB)
{
	// Store original positions
	const FIntPoint OriginA = MovingItemA.Value.Origin;
	const FIntPoint OriginB = MovingItemB.Value.Origin;

	// Get rotated shapes for both items
	FSpatialItemPlacement PlacementCopyA = MovingItemA.Value;
	FSpatialItemPlacement PlacementCopyB = MovingItemB.Value;

	// Check if both items would fit in their new positions
	PlacementCopyA.Origin = OriginB;
	PlacementCopyB.Origin = OriginA;

	// This is a first check mainly to see if the item would fit inside the grids bounds
	if (!FitsInGrid(PlacementCopyA, MakeArrayView(&MovingItemB.Key, 1)) ||
		!FitsInGrid(PlacementCopyB, MakeArrayView(&MovingItemA.Key, 1)))
	{
		return false;
	}

	// @todo this needs to be reimplemented to not MarkItemDirty if it fails

	UpdateItemPosition(MovingItemA, OriginB);
	UpdateItemPosition(MovingItemB, OriginA);

	// Check if both items can exist in their new positions without overlapping
	bool bValidSwap = true;
	for (const FIntPoint& Point : PlacementCopyA.ItemShape.Points)
	{
		for (const FIntPoint& OtherPoint : PlacementCopyB.ItemShape.Points)
		{
			if (Point + MovingItemA.Value.Origin == OtherPoint + MovingItemB.Value.Origin)
			{
				bValidSwap = false;
				break;
			}
		}
		if (!bValidSwap)
		{
			break;
		}
	}

	// Revert to original positions if validation fails
	if (!bValidSwap)
	{
		UpdateItemPosition(MovingItemA, OriginA);
		UpdateItemPosition(MovingItemB, OriginB);
		return false;
	}

	return true;
}

bool UInventorySpatialGridExtension::MoveSingleItem(FSpatialKeyedEntry& Item, const FIntPoint& NewPosition)
{
	FSpatialItemPlacement ItemPlacement = Item.Value;
	ItemPlacement.Origin = NewPosition;
	if (!FitsInGrid(ItemPlacement, MakeArrayView(&Item.Key, 1)))
	{
		return false;
	}

	UpdateItemPosition(Item, NewPosition);
	return true;
}

void UInventorySpatialGridExtension::UpdateItemPosition(FSpatialKeyedEntry& Item, const FIntPoint& NewPosition)
{
	// We could have the same index in both the add and removal so we need to clear first
	const FFaerieGridShape Rotated = Item.Value.GetRotated();

	// Clear old positions first
	for (auto& Point : Rotated.Points)
	{
		const FIntPoint OldPoint = Item.Value.Origin + Point;
		OccupiedCells[Ravel(OldPoint)] = false;
	}

	// Then set new positions
	for (auto& Point : Rotated.Points)
	{
		const FIntPoint Translated = NewPosition + Point;
		OccupiedCells[Ravel(Translated)] = true;
	}

	const FIntPoint Diff = NewPosition - Item.Value.Origin;
	Item.Value.Origin = NewPosition;
	Item.Value.PivotPoint = Item.Value.PivotPoint + Diff;
	SpatialEntries.MarkItemDirty(Item);
}

bool UInventorySpatialGridExtension::RotateItem(const FInventoryKey& Key)
{
	return SpatialEntries.EditItem(Key,
		[this, Key](FSpatialItemPlacement& Entry)
		{
			// No Point in Trying to Rotate
			if (Entry.ItemShape.IsSymmetrical()) return;

			const ESpatialItemRotation NextRotation = GetNextRotation(Entry.Rotation);

			FSpatialItemPlacement TempPlacementData = Entry;
			TempPlacementData.Rotation = NextRotation;
			if (!FitsInGrid(TempPlacementData, MakeArrayView(&Key, 1)))
			{
				return;
			}

			// Store old points before transformations so we can clear them from the bit grid
			const FFaerieGridShape OldShape = Entry.GetRotated();

			Entry.Rotation = NextRotation;

			// Clear old occupied cells
			for (const auto& OldPoint : OldShape.Points)
			{
				OccupiedCells[Ravel(OldPoint)] = false;
			}

			// Set new occupied cells taking into account rotation
			const FFaerieGridShape NewShape = Entry.GetRotated();
			for (const auto& Point : NewShape.Points)
			{
				const FIntPoint NewPoint = Entry.Origin + Point;
				OccupiedCells[Ravel(NewPoint)] = true;
			}
		});
}