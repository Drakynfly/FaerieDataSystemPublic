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

void FSpatialContent::Insert(FInventoryKey Key, const FSpatialItemPlacement& Value)
{
	FSpatialKeyedEntry& NewEntry = BSOA::Insert({Key, Value});
	MarkItemDirty(NewEntry);
}

void FSpatialContent::Remove(const FInventoryKey Key)
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
	for (const FFaerieItemKeyBase BaseKey : Event.OtherKeysTouched)
	{
		const FStackKey CurrentStackKey(BaseKey.Value());
		FInventoryKey NewKey;
		NewKey.EntryKey = Event.EntryTouched;
		NewKey.StackKey = CurrentStackKey;
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
	RemoveItemFromGrid(FInventoryKey());
}

void UInventorySpatialGridExtension::PreEntryReplicatedRemove(const FSpatialKeyedEntry& Entry)
{
	SpatialEntryChangedDelegateNative.Broadcast(Entry.Key);
	SpatialEntryChangedDelegate.Broadcast(Entry.Key);
}

void UInventorySpatialGridExtension::PostEntryReplicatedAdd(const FSpatialKeyedEntry& Entry)
{
	SpatialEntryChangedDelegateNative.Broadcast(Entry.Key);
	SpatialEntryChangedDelegate.Broadcast(Entry.Key);
}

void UInventorySpatialGridExtension::PostEntryReplicatedChange(const FSpatialKeyedEntry& Entry)
{
	SpatialEntryChangedDelegateNative.Broadcast(Entry.Key);
	SpatialEntryChangedDelegate.Broadcast(Entry.Key);
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

	if (OccupiedSlots.Find(Key) != nullptr)
	{
		return true;
	}

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

	OccupiedSlots.Insert(Key, Placement);
	return true;
}

void UInventorySpatialGridExtension::RemoveItemFromGrid(const FInventoryKey& Key)
{
	const TConstArrayView<FSpatialKeyedEntry> Entries = OccupiedSlots.GetEntries();
	for (int32 i = Entries.Num() - 1; i >= 0; --i)
	{
		if (Entries[i].Key == Key)
		{
			OccupiedSlots.Remove(Entries[i].Key);
		}
	}
}

FFaerieGridShape UInventorySpatialGridExtension::GetEntryShape(const FInventoryKey& Key) const
{
	if (auto&& Placement = OccupiedSlots.Find(Key))
	{
		return Placement->ItemShape;
	}

	// No entry,
	return FFaerieGridShape();
}

FSpatialItemPlacement UInventorySpatialGridExtension::GetEntryPlacementData(const FInventoryKey& Key) const
{
	if (auto&& Placement = OccupiedSlots.Find(Key))
	{
		return *Placement;
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
                                                const TConstArrayView<FInventoryKey> ExcludedKeys) const
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
			if (ExcludedKeys.Contains(Entry.Key))
			{
				continue;
			}

			// Create rotated copy of shape we are comparing against
			FFaerieGridShape LocalRotatedShape = Entry.Value.ItemShape;
			LocalRotatedShape.RotateAboutAngle(static_cast<float>(Entry.Value.Rotation) * 90.f);
			for (const FIntPoint& ExistingPoint : LocalRotatedShape.Points)
			{
				if (AbsolutePosition == (Entry.Value.Origin + ExistingPoint))
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
	FFaerieGridShape RotatedShape = MatchingEntry->Value.ItemShape;
	RotatedShape.RotateAboutAngle(static_cast<float>(MatchingEntry->Value.Rotation) * 90.f);

	if (FSpatialKeyedEntry* OverlappingItem =
		FindOverlappingItem(RotatedShape, Offset + MatchingEntry->Value.Origin, Key))
	{
		return TrySwapItems(*MatchingEntry, *OverlappingItem, Offset);
	}

	return MoveSingleItem(*MatchingEntry, Offset);
}

FSpatialKeyedEntry* UInventorySpatialGridExtension::FindItemByKey(const FInventoryKey& Key)
{
	return OccupiedSlots.Items.FindByPredicate([&Key](const FSpatialKeyedEntry& In)
	{
		return Key == In.Key;
	});
}

bool UInventorySpatialGridExtension::ValidateSourcePoint(const FSpatialKeyedEntry* Entry, const FIntPoint& SourcePoint)
{
	// Create a rotated version of the shape to validate against
	FFaerieGridShape RotatedShape = Entry->Value.ItemShape;
	RotatedShape.RotateAboutAngle(static_cast<float>(Entry->Value.Rotation) * 90.f);

	const FIntPoint LocalSourcePoint = SourcePoint - Entry->Value.Origin;
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
			if (ExcludeKey == In.Key)
				return false;

			// Create a rotated version of the "In" item's shape
			FFaerieGridShape OtherRotatedShape = In.Value.ItemShape;
			OtherRotatedShape.RotateAboutAngle(static_cast<float>(In.Value.Rotation) * 90.f);

			for (const FIntPoint& Point : Shape.Points)
			{
				const FIntPoint TranslatedPoint = Point + Offset;
				for (const FIntPoint& OtherPoint : OtherRotatedShape.Points)
				{
					if (TranslatedPoint == (In.Value.Origin + OtherPoint))
					{
						return true;
					}
				}
			}
			return false;
		});
}

bool UInventorySpatialGridExtension::TrySwapItems(FSpatialKeyedEntry& MovingItem, FSpatialKeyedEntry& OverlappingItem,
												  const FIntPoint& Offset)
{
	const FIntPoint ReverseOffset = FIntPoint(-Offset.X, -Offset.Y);

	// Store original positions incase validations fail and we need to reverse
	const FIntPoint OriginalMovingOrigin = MovingItem.Value.Origin;
	const FIntPoint OriginalOverlappingOrigin = OverlappingItem.Value.Origin;

	// Get rotated shapes for both items
	FFaerieGridShape MovingRotatedShape = MovingItem.Value.ItemShape;
	MovingRotatedShape.RotateAboutAngle(static_cast<float>(MovingItem.Value.Rotation) * 90.f);

	FFaerieGridShape OverlappingRotatedShape = OverlappingItem.Value.ItemShape;
	OverlappingRotatedShape.RotateAboutAngle(static_cast<float>(OverlappingItem.Value.Rotation) * 90.f);

	// Check if both items would fit in their new positions
	const FIntPoint NewMovingOrigin = OriginalMovingOrigin + Offset;
	const FIntPoint NewOverlappingOrigin = OriginalOverlappingOrigin + ReverseOffset;
	// This is a first check mainly to see if the item would fit inside the grids bounds
	if (!FitsInGrid(MovingRotatedShape, NewMovingOrigin, MovingItem.Value.Rotation, MakeArrayView(&OverlappingItem.Key, 1)) ||
		!FitsInGrid(OverlappingRotatedShape, NewOverlappingOrigin, OverlappingItem.Value.Rotation,
		            MakeArrayView(&MovingItem.Key, 1)))
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
			if ((Point + MovingItem.Value.Origin) == (OtherPoint + OverlappingItem.Value.Origin))
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
		MovingItem.Value.Origin = OriginalMovingOrigin;
		OverlappingItem.Value.Origin = OriginalOverlappingOrigin;
		return false;
	}

	OccupiedSlots.MarkItemDirty(MovingItem);
	OccupiedSlots.MarkItemDirty(OverlappingItem);

	return true;
}


bool UInventorySpatialGridExtension::MoveSingleItem(FSpatialKeyedEntry& Item, const FIntPoint& Offset)
{
	if (!FitsInGrid(Item.Value.ItemShape, Item.Value.Origin + Offset, Item.Value.Rotation, TArray{Item.Key}))
	{
		return false;
	}

	UpdateItemPosition(Item, Offset);
	return true;
}

void UInventorySpatialGridExtension::UpdateItemPosition(FSpatialKeyedEntry& Item, const FIntPoint& Offset)
{
	// @todo Drakyn: look at this
	Item.Value.Origin = Item.Value.Origin + Offset;
	Item.Value.PivotPoint = Item.Value.PivotPoint + Offset;
	OccupiedSlots.MarkItemDirty(Item);
}

ESpatialItemRotation GetNextRotation(const ESpatialItemRotation CurrentRotation)
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
		return Key == In.Key;
	});

	if (!MatchingEntry)
	{
		return false;
	}

	const ESpatialItemRotation NextRotation = GetNextRotation(MatchingEntry->Value.Rotation);
	if (!FitsInGrid(MatchingEntry->Value.ItemShape, MatchingEntry->Value.Origin, NextRotation, TArray{Key}))
	{
		return false;
	}

	MatchingEntry->Value.Rotation = NextRotation;

	OccupiedSlots.MarkItemDirty(*MatchingEntry);
	return true;
}