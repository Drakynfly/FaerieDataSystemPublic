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

void FSpatialContent::Insert(FSpatialEntryKey Key, FEntryKey Value)
{
	FSpatialKeyedEntry& NewEntry = BSOA::Insert({Key, Value});
	MarkItemDirty(NewEntry);
}

void FSpatialContent::Remove(const FSpatialEntryKey Key)
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
	if (const UFaerieShapeToken* ShapeToken = Event.Item->GetToken<UFaerieShapeToken>())
	{
		AddItemToGrid(Event.EntryTouched, ShapeToken);
	}
	else
	{
		AddItemToGrid(Event.EntryTouched, nullptr);
	}
}

void UInventorySpatialGridExtension::PostRemoval(const UFaerieItemContainerBase* Container,
                                                 const Faerie::Inventory::FEventLog& Event)
{
	RemoveItemFromGrid(Event.EntryTouched);
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

bool UInventorySpatialGridExtension::AddItemToGrid(const FEntryKey& Key, const UFaerieShapeToken* ShapeToken)
{
	if (!Key.IsValid())
	{
		return false;
	}

	FFaerieGridShape Shape;

	if (ShapeToken)
	{
		Shape = ShapeToken->GetShape();
	}
	else
	{
		// Default for Items with no Shape Token.
		Shape = FFaerieGridShape::MakeRect(1, 1);
	}

	// @todo this should check for possible rotations! (but only if the shape is different when its rotated)
	TOptional<FIntPoint> FoundPosition = GetFirstEmptyLocation(Shape);

	// Nowhere to put this item
	if (!FoundPosition.IsSet()) return false;

	// Move the shape to its position
	Shape.TranslateInline(FoundPosition.GetValue());

	for (const FIntPoint& Pos : Shape.Points)
	{
		OccupiedSlots.Insert(FSpatialEntryKey{Pos}, Key);
	}

	return true;
}

void UInventorySpatialGridExtension::RemoveItemFromGrid(const FEntryKey& Key)
{
	const auto& Entries = OccupiedSlots.GetEntries();
	for (int32 i = Entries.Num() - 1; i >= 0; --i)
	{
		if (Entries[i].Value == Key)
		{
			OccupiedSlots.Remove(Entries[i].Key);
		}
	}
}

FFaerieGridShape UInventorySpatialGridExtension::GetEntryPositions(const FEntryKey& Key) const
{
	FFaerieGridShape PositionsToReturn;
	for (const FSpatialKeyedEntry& Entry : OccupiedSlots.GetEntries())
	{
		if (Entry.Value == Key)
		{
			PositionsToReturn.Points.Add(Entry.Key.Key);
		}
	}
	return PositionsToReturn;
}

bool UInventorySpatialGridExtension::FitsInGrid(const FFaerieGridShape& Shape, const FIntPoint& Position,
												const bool bCheckingRotation, const FEntryKey& ExcludedKey) const
{
	const FFaerieGridShape LocalShape = Shape.Normalize();
	for (const FIntPoint& Coord : LocalShape.Points)
	{
		const FIntPoint AbsolutePosition = Position + Coord;

		if (AbsolutePosition.X < 0 || AbsolutePosition.X >= GridSize.X ||
			AbsolutePosition.Y < 0 || AbsolutePosition.Y >= GridSize.Y)
		{
			return false;
		}

		for (const FSpatialKeyedEntry& Entry : OccupiedSlots.GetEntries())
		{
			if (Entry.Key.Key == AbsolutePosition)
			{
				if (bCheckingRotation && Entry.Value == ExcludedKey)
				{
					continue;
				}
				return false;
			}
		}
	}
	return true;
}


TOptional<FIntPoint> UInventorySpatialGridExtension::GetFirstEmptyLocation(const FFaerieGridShape& InShape) const
{
	FIntPoint TestPosition = FIntPoint::ZeroValue;
	for (; TestPosition.Y < GridSize.Y; TestPosition.Y++)
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

bool UInventorySpatialGridExtension::MoveItem(const FEntryKey& Key, const FIntPoint& SourcePoint,
                                              const FIntPoint& TargetPoint)
{
	const FFaerieGridShape CurrentShape = GetEntryPositions(Key);
	if (CurrentShape.Points.IsEmpty() ||
		!CurrentShape.Points.Contains(SourcePoint)) //@todo this line is wrong!
	{
		return false;
	}

	const FIntPoint Offset = TargetPoint - SourcePoint;

	const FFaerieGridShape NewShape = CurrentShape.Translate(Offset);

	// NewShape is already translated, so check against position zero to offset the translation.
	if (!FitsInGrid(NewShape, FIntPoint::ZeroValue))
	{
		return false;
	}

	// Remove old points
	RemoveItemFromGrid(Key);

	// Add new points
	for (const FIntPoint& Pos : NewShape.Points)
	{
		OccupiedSlots.Insert(FSpatialEntryKey{Pos}, Key);
	}

	return true;
}

bool UInventorySpatialGridExtension::RotateItem(const FEntryKey& Key, const FIntPoint& PivotPoint)
{
	const FFaerieGridShape NewShape = GetEntryPositions(Key).Rotate(PivotPoint);

	if (!FitsInGrid(NewShape, PivotPoint, true, Key)) return false;

	// Remove old points
	RemoveItemFromGrid(Key);

	// Add new points
	for (const FIntPoint& Pos : NewShape.Points)
	{
		OccupiedSlots.Insert(FSpatialEntryKey{Pos}, Key);
	}

	return true;
}