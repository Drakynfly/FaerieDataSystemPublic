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
	Sort();
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

	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, OccupiedSlots, SharedParams);
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, GridSize, SharedParams);
}

void UInventorySpatialGridExtension::PostInitProperties()
{
	Super::PostInitProperties();
	OccupiedSlots.ChangeListener = this;
}

void UInventorySpatialGridExtension::InitializeExtension(const UFaerieItemContainerBase* Container)
{
	// Add all existing items to the grid on startup.
	// This is dumb, and just adds them in order, it doesn't space pack them. To do that, we would want to sort items by size, and add largest first.
	// This is also skipping possible serialization of grid data.
	// @todo handle serialization loading
	// @todo handle items that are too large to fix / too many items (log error?)

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
	// @todo don't remove items when the stacks are only partially removed

	FInventoryKey Key;
	Key.EntryKey = Event.EntryTouched;
	if (const UFaerieItemStorage* ItemStorage = Cast<UFaerieItemStorage>(Container))
	{
		const TArray<FInventoryKey> InvKeys = ItemStorage->GetInvKeysForEntry(Key.EntryKey);
		for (const FStackKey& StackKey : Event.StackKeys)
		{
			Key.StackKey = StackKey;
			if (!ItemStorage->IsValidKey(InvKeys[InvKeys.IndexOfByKey(Key)]))
			{
				RemoveItem(Key);
			}
		}
	}
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
	bool RetVal = false;
	ESpatialItemRotation Rotation = ESpatialItemRotation::None;
	const int32 RotationsToCheck = ShapeToken->GetShape().bIsSymmetrical ? 1 : 4;
	for (int i = 0; i < RotationsToCheck; i++)
	{
		if (FitsInGrid(ShapeToken->GetShape(), Position, Rotation))
		{
			RetVal = true;
			break;
		}
		Rotation = GetNextRotation(Rotation);
	}
	return RetVal;
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
	TOptional<TTuple<FIntPoint, ESpatialItemRotation>> FoundPosition = GetFirstEmptyLocation(Shape);

	if (!FoundPosition.IsSet()) return false;

	FSpatialItemPlacement Placement;
	Placement.Origin = FoundPosition.GetValue().Key;
	Placement.ItemShape = Shape;
	Placement.PivotPoint = Shape.GetShapeCenter() + FoundPosition.GetValue().Key;
	Placement.Rotation = FoundPosition.GetValue().Value;

	OccupiedSlots.Insert(Key, Placement);
	return true;
}

void UInventorySpatialGridExtension::RemoveItem(const FInventoryKey& Key)
{
	OccupiedSlots.Remove(Key);
}

void UInventorySpatialGridExtension::RemoveItemsForEntry(const FEntryKey& Key)
{
	TArray<FInventoryKey> Keys;
	for (auto&& Element : OccupiedSlots)
	{
		if (Element.Key.EntryKey == Key)
		{
			Keys.Add(Element.Key);
		}
	}

	for (auto&& InvKey : Keys)
	{
		RemoveItem(InvKey);
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
	// Create rotated copy of input shape
	FFaerieGridShape RotatedShape = Shape;
	RotatedShape.RotateAboutAngle(static_cast<float>(Rotation) * 90.f);

	// Check rotated shape points
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
		for (const FSpatialKeyedEntry& Entry : OccupiedSlots)
		{
			// Skip if this is an excluded item (e.g., the item being moved/rotated)
			if (ExcludedKeys.Contains(Entry.Key))
			{
				continue;
			}

			FFaerieGridShape ShapeToCheck = Entry.Value.ItemShape;
			ShapeToCheck.RotateAboutAngle(static_cast<float>(Entry.Value.Rotation) * 90.f);
			for (const FIntPoint& ExistingPoint : ShapeToCheck.Points)
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

TOptional<TTuple<FIntPoint, ESpatialItemRotation>> UInventorySpatialGridExtension::GetFirstEmptyLocation(
	const FFaerieGridShape& InShape) const
{
	// Try each rotation
	const int32 RotationsToCheck = InShape.bIsSymmetrical ? 1 : 4;
	for (int32 RotIndex = 0; RotIndex < RotationsToCheck; RotIndex++)
	{
		ESpatialItemRotation Rotation = static_cast<ESpatialItemRotation>(RotIndex);
		// Check each grid position at each rotation
		for (int32 Y = 0; Y < GridSize.Y; Y++)
		{
			for (int32 X = 0; X < GridSize.X; X++)
			{
				if (FIntPoint TestPosition(X, Y); FitsInGrid(InShape, TestPosition, Rotation))
				{
					return MakeTuple(TestPosition, Rotation);
				}
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
	if (const int32 Index = OccupiedSlots.IndexOf(Key);
		Index != INDEX_NONE)
	{
		return &OccupiedSlots.Items[Index];
	}
	return nullptr;
}

bool UInventorySpatialGridExtension::ValidateSourcePoint(const FSpatialKeyedEntry* Entry, const FIntPoint& SourcePoint)
{
	// Create a rotated version of the shape to validate against
	FFaerieGridShape RotatedShape = Entry->Value.ItemShape;
	RotatedShape.RotateAboutAngle(static_cast<float>(Entry->Value.Rotation) * 90.f);

	const FIntPoint LocalSourcePoint = SourcePoint - Entry->Value.Origin;
	return RotatedShape.Points.Contains(LocalSourcePoint);
}

FSpatialKeyedEntry* UInventorySpatialGridExtension::FindOverlappingItem(const FFaerieGridShape& Shape,
                                                                        const FIntPoint& Offset,
                                                                        const FInventoryKey& ExcludeKey)
{
	return OccupiedSlots.Items.FindByPredicate(
		[this, &Shape, &Offset, ExcludeKey](const FSpatialKeyedEntry& In)
		{
			if (ExcludeKey == In.Key) return false;

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

	// Store original positions in case validations fail, and we need to reverse
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
	if (!FitsInGrid(MovingRotatedShape, NewMovingOrigin, MovingItem.Value.Rotation,
	                MakeArrayView(&OverlappingItem.Key, 1)) ||
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
	if (!FitsInGrid(Item.Value.ItemShape, Item.Value.Origin + Offset, Item.Value.Rotation, MakeArrayView(&Item.Key, 1)))
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

bool UInventorySpatialGridExtension::RotateItem(const FInventoryKey& Key)
{
	return OccupiedSlots.EditItem(Key,
	                              [this, Key](FSpatialItemPlacement& Entry)
	                              {
		                              const ESpatialItemRotation NextRotation = GetNextRotation(Entry.Rotation);
		                              if (!FitsInGrid(Entry.ItemShape, Entry.Origin, NextRotation,
		                                              MakeArrayView(&Key, 1)) || Entry.ItemShape.bIsSymmetrical)
		                              {
			                              return;
		                              }

		                              Entry.Rotation = NextRotation;
	                              });
}
