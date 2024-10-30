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
	DOREPLIFETIME(ThisClass, GridSize);
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

bool UInventorySpatialGridExtension::CanAddItemToGrid(const UFaerieShapeToken* ShapeToken,
                                                      const FIntPoint& Position) const
{
	if (!ShapeToken) return false;
	return FitsInGrid(ShapeToken->GetShape(), Position, OccupiedSlots);
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

	if (!ShapeToken)
	{
		FFaerieGridShape NewShape = FFaerieGridShape::MakeRect(1, 1);
		const FSpatialEntryKey EntryKey = {NewShape.Points[0]};
		OccupiedSlots.Insert(EntryKey, Key);
		return true;
	}

	TOptional<FIntPoint> FoundPosition = GetFirstEmptyLocation(ShapeToken->GetShape(), OccupiedSlots);
	if (!FoundPosition.IsSet()) return false;
	FFaerieGridShape Shape = ShapeToken->GetShape();
	Shape.Translate(FoundPosition.GetValue());
	for (const FIntPoint& Pos : Shape.Points)
	{
		const FSpatialEntryKey EntryKey{Pos};
		OccupiedSlots.Insert(EntryKey, Key);
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
                                                const FSpatialContent& Occupied) const
{
	FFaerieGridShape LocalShape = Shape;
	LocalShape.NormalizeShape();
	for (const FIntPoint& Coord : LocalShape.Points)
	{
		const FIntPoint AbsolutePosition = Position + Coord;

		if (AbsolutePosition.X < 0 || AbsolutePosition.X >= GridSize.X ||
			AbsolutePosition.Y < 0 || AbsolutePosition.Y >= GridSize.Y)
		{
			return false;
		}

		for (const FSpatialKeyedEntry& Entry : Occupied.GetEntries())
		{
			if (Entry.Key.Key == AbsolutePosition)
			{
				return false;
			}
		}
	}
	return true;
}


TOptional<FIntPoint> UInventorySpatialGridExtension::GetFirstEmptyLocation(
	const FFaerieGridShape& InShape, const FSpatialContent& Occupied) const
{
	FIntPoint TestPosition = FIntPoint::ZeroValue;
	for (; TestPosition.Y < GridSize.Y; TestPosition.Y++)
	{
		for (; TestPosition.X < GridSize.X; TestPosition.X++)
		{
			if (FitsInGrid(InShape, TestPosition, Occupied))
			{
				return TestPosition;
			}
		}
	}
	return NullOpt;
}

void UInventorySpatialGridExtension::SetGridSize(FIntPoint NewGridSize)
{
	GridSize = NewGridSize;
}

bool UInventorySpatialGridExtension::MoveItem(const FEntryKey& Key, const FIntPoint& SourcePoint,
                                              const FIntPoint& TargetPoint)
{
	FFaerieGridShape CurrentShape = GetEntryPositions(Key);
	if (CurrentShape.Points.Num() == 0)
	{
		return false;
	}

	if (!CurrentShape.Points.Contains(SourcePoint))
	{
		return false;
	}

	FIntPoint Offset = TargetPoint - SourcePoint;

	if (!CanMoveItem(CurrentShape, Offset))
	{
		return false;
	}

	RemoveItemFromGrid(Key);

	FFaerieGridShape NewShape;
	for (const FIntPoint& Point : CurrentShape.Points)
	{
		NewShape.Points.Add(Point + Offset);
	}

	for (const FIntPoint& Pos : NewShape.Points)
	{
		const FSpatialEntryKey EntryKey{Pos};
		OccupiedSlots.Insert(EntryKey, Key);
	}

	return true;
}

bool UInventorySpatialGridExtension::RotateItem(const FEntryKey& Key, const FIntPoint& PivotPoint)
{
	FFaerieGridShape CurrentShape = GetEntryPositions(Key);
	if (CurrentShape.Points.Num() == 0 || CurrentShape.Points.IsEmpty())
	{
		return false;
	}

	if(Key.Value() == -1) return false;

	if (!CurrentShape.Points.Contains(PivotPoint))
	{
		return false;
	}
	
	if(!CanRotateItem(CurrentShape, PivotPoint)) return false;

	RemoveItemFromGrid(Key);

	FFaerieGridShape NewShape;
	NewShape.Points.Reserve(CurrentShape.Points.Num());
	for (const FIntPoint& Point : CurrentShape.Points)
	{
		FIntPoint Relative = Point - PivotPoint;
		FIntPoint Rotated(Relative.Y, -Relative.X);
		NewShape.Points.Add(Rotated + PivotPoint);
	}

	for (const FIntPoint& Pos : NewShape.Points)
	{
		const FSpatialEntryKey EntryKey{Pos};
		OccupiedSlots.Insert(EntryKey, Key);
	}
	return true;
}

bool UInventorySpatialGridExtension::CanMoveItem(const FFaerieGridShape& Shape, const FIntPoint& Offset) const
{
	FFaerieGridShape NewShape;
	for (const FIntPoint& Point : Shape.Points)
	{
		FIntPoint NewPoint = Point + Offset;

		if (NewPoint.X < 0 || NewPoint.X >= GridSize.X ||
			NewPoint.Y < 0 || NewPoint.Y >= GridSize.Y)
		{
			return false;
		}

		const FSpatialEntryKey EntryKey{NewPoint};
		for (const FSpatialKeyedEntry& Entry : OccupiedSlots.GetEntries())
		{
			if (Entry.Key == EntryKey && !Shape.Points.Contains(Entry.Key.Key))
			{
				return false;
			}
		}

		NewShape.Points.Add(NewPoint);
	}

	return true;
}

bool UInventorySpatialGridExtension::CanRotateItem(const FFaerieGridShape& Shape, const FIntPoint& PivotPoint) const
{
	for (const FIntPoint& Point : Shape.Points)
	{
		FIntPoint Relative = Point - PivotPoint;
		FIntPoint Rotated(Relative.Y, -Relative.X);
		FIntPoint NewPoint = Rotated + PivotPoint;

		if (NewPoint.X < 0 || NewPoint.X >= GridSize.X ||
		   NewPoint.Y < 0 || NewPoint.Y >= GridSize.Y)
		{
			return false;
		}

		const FSpatialEntryKey EntryKey{NewPoint};
		for (const FSpatialKeyedEntry& Entry : OccupiedSlots.GetEntries())
		{
			if (Entry.Key == EntryKey && !Shape.Points.Contains(Entry.Key.Key))
			{
				return false;
			}
		}
	}

	return true;
}

void UInventorySpatialGridExtension::OnRep_GridSize()
{
	GridSizeChangedDelegateNative.Broadcast(GridSize);
	GridSizeChangedDelegate.Broadcast(GridSize);
}
