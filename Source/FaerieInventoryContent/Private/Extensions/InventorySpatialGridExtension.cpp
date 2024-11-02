// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "Extensions/InventorySpatialGridExtension.h"

#include "FaerieItemContainerBase.h"
#include "ItemContainerEvent.h"
#include "Net/UnrealNetwork.h"
#include "Tokens/FaerieShapeToken.h"

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

void FSpatialContent::PreEntryReplicatedRemove(const FSpatialKeyedEntry& Entry) const
{
	ChangeListener->PreEntryReplicatedRemove(Entry);
}

void FSpatialContent::PostEntryReplicatedAdd(const FSpatialKeyedEntry& Entry)
{
	ChangeListener->PostEntryReplicatedAdd(Entry);
	Sort();
}

void FSpatialContent::PostEntryReplicatedChange(const FSpatialKeyedEntry& Entry) const
{
	ChangeListener->PostEntryReplicatedChange(Entry);
}

void FSpatialContent::Insert(FSpatialItemPlacement Key, FInventoryKey Value)
{
	FSpatialKeyedEntry& NewEntry = BSOA::Insert({Key, Value});
	MarkItemDirty(NewEntry);
}

void FSpatialContent::Remove(const FSpatialItemPlacement Key)
{
	if (BSOA::Remove(Key))
	{
		MarkArrayDirty();
	}
}

void UInventorySpatialGridExtension::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams SharedParams;
	SharedParams.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, OccupiedSlots, SharedParams);
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, GridSize, SharedParams);
}

void UInventorySpatialGridExtension::PostInitProperties()
{
	Super::PostInitProperties();
	OccupiedSlots.ChangeListener = this;
}

EEventExtensionResponse UInventorySpatialGridExtension::AllowsAddition(const UFaerieItemContainerBase* Container,
                                                                       const FFaerieItemStackView Stack)
{
	if (!CanAddItemToGrid(Stack.Item->GetToken<UFaerieShapeToken>()))
	{
		return EEventExtensionResponse::Disallowed;
	}

	return EEventExtensionResponse::Allowed;
}

void UInventorySpatialGridExtension::PostAddition(const UFaerieItemContainerBase* Container,
                                                  const Faerie::Inventory::FEventLog& Event)
{
	const FStackKey BaseStackKey(Event.OtherKeysTouched[0].Value());
	FInventoryKey NewKey;
	NewKey.EntryKey = Event.EntryTouched;
	NewKey.StackKey = BaseStackKey;
	if (const UFaerieShapeToken* ShapeToken = Event.Item->GetToken<UFaerieShapeToken>())
	{
		AddItemToGrid(NewKey, ShapeToken);
	}
	else
	{
		AddItemToGrid(NewKey, nullptr);
	}
}

void UInventorySpatialGridExtension::PostRemoval(const UFaerieItemContainerBase* Container,
                                                 const Faerie::Inventory::FEventLog& Event)
{
	RemoveItemFromGrid(FInventoryKey());
}

void UInventorySpatialGridExtension::PreEntryReplicatedRemove(const FSpatialKeyedEntry& Entry)
{
	SpatialEntryChangedDelegateNative.Broadcast(Entry.Value);
	SpatialEntryChangedDelegate.Broadcast(Entry.Value);
}

void UInventorySpatialGridExtension::PostEntryReplicatedAdd(const FSpatialKeyedEntry& Entry)
{
	SpatialEntryChangedDelegateNative.Broadcast(Entry.Value);
	SpatialEntryChangedDelegate.Broadcast(Entry.Value);
}

void UInventorySpatialGridExtension::PostEntryReplicatedChange(const FSpatialKeyedEntry& Entry)
{
	SpatialEntryChangedDelegateNative.Broadcast(Entry.Value);
	SpatialEntryChangedDelegate.Broadcast(Entry.Value);
}

void UInventorySpatialGridExtension::OnRep_GridSize()
{
	GridSizeChangedDelegateNative.Broadcast(GridSize);
	GridSizeChangedDelegate.Broadcast(GridSize);
}

bool UInventorySpatialGridExtension::CanAddItemToGrid(const UFaerieShapeToken* ShapeToken,
                                                      const FIntPoint& Position) const
{
	if (!ShapeToken) return false;
	return FitsInGrid(ShapeToken->GetShape(), Position);
}

bool UInventorySpatialGridExtension::CanAddItemToGrid(const UFaerieShapeToken* ShapeToken) const
{
	if (!ShapeToken) return true;

	for (int32 x = 0; x < GridSize.X; x++)
	{
		for (int32 y = 0; y < GridSize.Y; y++)
		{
			if (CanAddItemToGrid(ShapeToken, FIntPoint(x, y)))
			{
				return true;
			}
		}
	}

	return false;
}

bool UInventorySpatialGridExtension::AddItemToGrid(const FInventoryKey& Key, const UFaerieShapeToken* ShapeToken)
{
	if (!Key.IsValid()) return false;

	FFaerieGridShape Shape;
	if (ShapeToken)
	{
		Shape = ShapeToken->GetShape();
	}
	else
	{
		Shape = FFaerieGridShape::MakeRect(1, 1);
	}
	
	// @todo this should check for possible rotations! (but only if the shape is different when its rotated)
	TOptional<FIntPoint> FoundPosition = GetFirstEmptyLocation(Shape);

	if (!FoundPosition.IsSet()) return false;

	FSpatialItemPlacement Placement;
	Placement.Origin = FoundPosition.GetValue();
	Placement.ItemShape = Shape;
	Placement.PivotPoint = Shape.GetShapeCenter() + FoundPosition.GetValue();
	Placement.Rotation = ESpatialItemRotation::None;

	OccupiedSlots.Insert(Placement, Key);
	return true;
}

void UInventorySpatialGridExtension::RemoveItemFromGrid(const FInventoryKey& Key)
{
	const TConstArrayView<FSpatialKeyedEntry> Entries = OccupiedSlots.GetEntries();
	for (int32 i = Entries.Num() - 1; i >= 0; --i)
	{
		if (Entries[i].Value == Key)
		{
			OccupiedSlots.Remove(Entries[i].Key);
		}
	}
}

FFaerieGridShape UInventorySpatialGridExtension::GetEntryShape(const FInventoryKey& Key) const
{
	FFaerieGridShape PositionsToReturn;
	for (const FSpatialKeyedEntry& Entry : OccupiedSlots.GetEntries())
	{
		if (Entry.Value == Key)
		{
			return Entry.Key.ItemShape;
		}
	}
	return PositionsToReturn;
}

FSpatialItemPlacement UInventorySpatialGridExtension::GetEntryPlacementData(const FInventoryKey& Key) const
{
	for (const FSpatialKeyedEntry& Entry : OccupiedSlots.GetEntries())
	{
		if (Entry.Value == Key)
		{
			return Entry.Key;
		}
	}
	return FSpatialItemPlacement();
}

FFaerieGridShape UInventorySpatialGridExtension::RotateShape(FFaerieGridShape InShape,
                                                             const ESpatialItemRotation Rotation)
{
	InShape.RotateAboutAngle(static_cast<float>(Rotation) * 90.f);
	return InShape;
}

bool UInventorySpatialGridExtension::FitsInGrid(const FFaerieGridShape& Shape, const FIntPoint& Position,
                                                ESpatialItemRotation Rotation,
                                                const TArray<FInventoryKey>& ExcludedKeys) const
{
	// Create a copy of the shape for rotation and rotate around pivot point
	FFaerieGridShape RotatedShape = Shape;
	RotatedShape.RotateAboutAngle(static_cast<float>(Rotation) * 90.f);
	for (const FIntPoint& Coord : RotatedShape.Points)
	{
		const FIntPoint AbsolutePosition = Position + Coord;

		// Check grid boundaries
		if (AbsolutePosition.X < 0 || AbsolutePosition.X >= GridSize.X ||
			AbsolutePosition.Y < 0 || AbsolutePosition.Y >= GridSize.Y)
		{
			return false;
		}

		// Check for overlaps with existing items
		for (const FSpatialKeyedEntry& Entry : OccupiedSlots.GetEntries())
		{
			// Skip if this is an excluded item (e.g., the item being moved/rotated)
			if (ExcludedKeys.Contains(Entry.Value))
			{
				continue;
			}

			// Create rotated copy of shape we are comparing against
			FFaerieGridShape LocalRotatedShape = Entry.Key.ItemShape;
			LocalRotatedShape.RotateAboutAngle(static_cast<float>(Entry.Key.Rotation) * 90.f);
			for (const FIntPoint& ExistingPoint : LocalRotatedShape.Points)
			{
				if (AbsolutePosition == (Entry.Key.Origin + ExistingPoint))
				{
					return false;
				}
			}
		}
	}

	return true;
}


TOptional<FIntPoint> UInventorySpatialGridExtension::GetFirstEmptyLocation(const FFaerieGridShape& InShape) const
{
	for (FIntPoint TestPosition = FIntPoint::ZeroValue; TestPosition.Y < GridSize.Y; TestPosition.Y++)
	{
		for (; TestPosition.X < GridSize.X; TestPosition.X++)
		{
			if (FitsInGrid(InShape, TestPosition))
			{
				return TestPosition;
			}
		}
	}
	return NullOpt;
}

void UInventorySpatialGridExtension::SetGridSize(const FIntPoint NewGridSize)
{
	if (GridSize != NewGridSize)
	{
		GridSize = NewGridSize;
		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, GridSize, this);
	}
}

bool UInventorySpatialGridExtension::MoveItem(const FInventoryKey& Key, const FIntPoint& SourcePoint,
                                              const FIntPoint& TargetPoint)
{
	FSpatialKeyedEntry* MatchingEntry = FindItemByKey(Key);
	if (!MatchingEntry || !ValidateSourcePoint(MatchingEntry, SourcePoint))
	{
		return false;
	}
	const FIntPoint Offset = TargetPoint - SourcePoint;

	// Get the rotated shape based on current entry rotation so we can correctly get items that would overlap
	FFaerieGridShape RotatedShape = MatchingEntry->Key.ItemShape;
	RotatedShape.RotateAboutAngle(static_cast<float>(MatchingEntry->Key.Rotation) * 90.f);

	if (FSpatialKeyedEntry* OverlappingItem =
		FindOverlappingItem(RotatedShape, Offset + MatchingEntry->Key.Origin, Key))
	{
		return TrySwapItems(MatchingEntry, OverlappingItem, Offset);
	}

	return MoveSingleItem(MatchingEntry, Offset);
}

FSpatialKeyedEntry* UInventorySpatialGridExtension::FindItemByKey(const FInventoryKey& Key)
{
	return OccupiedSlots.Items.FindByPredicate([&Key](const FSpatialKeyedEntry& In)
	{
		return Key == In.Value;
	});
}

bool UInventorySpatialGridExtension::ValidateSourcePoint(const FSpatialKeyedEntry* Entry, const FIntPoint& SourcePoint)
{
	// Create a rotated version of the shape to validate against
	FFaerieGridShape RotatedShape = Entry->Key.ItemShape;
	RotatedShape.RotateAboutAngle(static_cast<float>(Entry->Key.Rotation) * 90.f);

	const FIntPoint LocalSourcePoint = SourcePoint - Entry->Key.Origin;
	return RotatedShape.Points.Contains(LocalSourcePoint);
}

FSpatialKeyedEntry* UInventorySpatialGridExtension::FindOverlappingItem(
	const FFaerieGridShape& Shape,
	const FIntPoint& Offset,
	const FInventoryKey& ExcludeKey)
{
	return OccupiedSlots.Items.FindByPredicate(
		[this, &Shape, &Offset, &ExcludeKey](const FSpatialKeyedEntry& In)
		{
			if (ExcludeKey == In.Value)
				return false;

			// Create a rotated version of the "In" item's shape 
			FFaerieGridShape OtherRotatedShape = In.Key.ItemShape;
			OtherRotatedShape.RotateAboutAngle(static_cast<float>(In.Key.Rotation) * 90.f);

			for (const FIntPoint& Point : Shape.Points)
			{
				const FIntPoint TranslatedPoint = Point + Offset;
				for (const FIntPoint& OtherPoint : OtherRotatedShape.Points)
				{
					if (TranslatedPoint == (In.Key.Origin + OtherPoint))
					{
						return true;
					}
				}
			}
			return false;
		});
}

bool UInventorySpatialGridExtension::TrySwapItems(
	FSpatialKeyedEntry* MovingItem,
	FSpatialKeyedEntry* OverlappingItem,
	const FIntPoint& Offset)
{
	const FIntPoint ReverseOffset = FIntPoint(-Offset.X, -Offset.Y);

	// Store original positions incase validations fail and we need to reverse
	const FIntPoint OriginalMovingOrigin = MovingItem->Key.Origin;
	const FIntPoint OriginalOverlappingOrigin = OverlappingItem->Key.Origin;

	// Get rotated shapes for both items
	FFaerieGridShape MovingRotatedShape = MovingItem->Key.ItemShape;
	MovingRotatedShape.RotateAboutAngle(static_cast<float>(MovingItem->Key.Rotation) * 90.f);

	FFaerieGridShape OverlappingRotatedShape = OverlappingItem->Key.ItemShape;
	OverlappingRotatedShape.RotateAboutAngle(static_cast<float>(OverlappingItem->Key.Rotation) * 90.f);

	// Check if both items would fit in their new positions
	const FIntPoint NewMovingOrigin = OriginalMovingOrigin + Offset;
	const FIntPoint NewOverlappingOrigin = OriginalOverlappingOrigin + ReverseOffset;
	// This is a first check mainly to see if the item would fit inside the grids bounds
	if (!FitsInGrid(MovingRotatedShape, NewMovingOrigin, MovingItem->Key.Rotation, TArray{OverlappingItem->Value}) ||
		!FitsInGrid(OverlappingRotatedShape, NewOverlappingOrigin, OverlappingItem->Key.Rotation,
		            TArray{MovingItem->Value}))
	{
		return false;
	}

	UpdateItemPosition(MovingItem, Offset);
	UpdateItemPosition(OverlappingItem, ReverseOffset);

	// Check if both items can exist in their new positions without overlapping
	bool bValidSwap = true;
	for (const FIntPoint& Point : MovingRotatedShape.Points)
	{
		for (const FIntPoint& OtherPoint : OverlappingRotatedShape.Points)
		{
			if ((Point + MovingItem->Key.Origin) == (OtherPoint + OverlappingItem->Key.Origin))
			{
				bValidSwap = false;
				break;
			}
		}
		if (!bValidSwap) break;
	}

	// Revert to original positions if validation fails
	if (!bValidSwap)
	{
		MovingItem->Key.Origin = OriginalMovingOrigin;
		OverlappingItem->Key.Origin = OriginalOverlappingOrigin;
		return false;
	}

	OccupiedSlots.MarkItemDirty(*MovingItem);
	OccupiedSlots.MarkItemDirty(*OverlappingItem);

	return true;
}


bool UInventorySpatialGridExtension::MoveSingleItem(FSpatialKeyedEntry* Item, const FIntPoint& Offset)
{
	if (!FitsInGrid(Item->Key.ItemShape, Item->Key.Origin + Offset, Item->Key.Rotation, TArray{Item->Value}))
	{
		return false;
	}

	UpdateItemPosition(Item, Offset);
	return true;
}

void UInventorySpatialGridExtension::UpdateItemPosition(FSpatialKeyedEntry* Item, const FIntPoint& Offset)
{
	Item->Key.Origin = Item->Key.Origin + Offset;
	Item->Key.PivotPoint = Item->Key.PivotPoint + Offset;
	OccupiedSlots.MarkItemDirty(*Item);
}

ESpatialItemRotation GetNextRotation(ESpatialItemRotation CurrentRotation)
{
	switch (CurrentRotation)
	{
	case ESpatialItemRotation::None:
		return ESpatialItemRotation::Ninety;
	case ESpatialItemRotation::Ninety:
		return ESpatialItemRotation::One_Eighty;
	case ESpatialItemRotation::One_Eighty:
		return ESpatialItemRotation::Two_Seventy;
	case ESpatialItemRotation::Two_Seventy:
		return ESpatialItemRotation::None;
	default:
		return ESpatialItemRotation::None;
	}
}

bool UInventorySpatialGridExtension::RotateItem(const FInventoryKey& Key)
{
	FSpatialKeyedEntry* MatchingEntry = OccupiedSlots.Items.FindByPredicate([&Key](const FSpatialKeyedEntry& In)
	{
		return Key == In.Value;
	});

	if (!MatchingEntry)
	{
		return false;
	}

	const ESpatialItemRotation NextRotation = GetNextRotation(MatchingEntry->Key.Rotation);
	if (!FitsInGrid(MatchingEntry->Key.ItemShape, MatchingEntry->Key.Origin, NextRotation, TArray{Key}))
	{
		return false;
	}

	MatchingEntry->Key.Rotation = NextRotation;

	OccupiedSlots.MarkItemDirty(*MatchingEntry);
	return true;
}
