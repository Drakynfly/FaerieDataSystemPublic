// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.


#include "Extensions/InventorySpatialGridExtension.h"

#include "FaerieItemContainerBase.h"
#include "ItemContainerEvent.h"
#include "Net/UnrealNetwork.h"
#include "Tokens/FaerieShapeToken.h"

FSpatialKeyedEntry::FSpatialKeyedEntry()
{
}

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
	//Todo
}

void FSpatialContent::PostEntryReplicatedAdd(const FSpatialKeyedEntry& Entry)
{
	ChangeListener->SpatialEntryChangedDelegate.Broadcast(Entry.Value);
	Sort();
}

void FSpatialContent::PostEntryReplicatedChange(const FSpatialKeyedEntry& Entry) const
{
	//Todo
}

void FSpatialContent::Insert(FSpatialEntryKey Key, const FEntryKey& Value)
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

void UInventorySpatialGridExtension::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams SharedParams;
	SharedParams.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, OccupiedSlots, SharedParams);
}

bool UInventorySpatialGridExtension::CanAddItemToGrid(const UFaerieShapeToken* ShapeToken,
                                                      const FIntPoint& Position) const
{
	if (!ShapeToken) return false;
	return ShapeToken->FitsInGrid(GridSize, Position, OccupiedSlots);
}

bool UInventorySpatialGridExtension::CanAddItemToGrid(const UFaerieShapeToken* ShapeToken) const
{
	if (!ShapeToken) return false;
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

bool UInventorySpatialGridExtension::AddItemToGrid(const FEntryKey& Key, const UFaerieShapeToken* ShapeToken,
                                                   const FIntPoint& Position)
{
	if (!ShapeToken)
	{
		return false;
	}

	auto FoundPosition = ShapeToken->GetFirstEmptyLocation(GridSize, OccupiedSlots);
	if (FoundPosition == FIntPoint(-1, -1)) return false;
	FFaerieGridShape Positions = ShapeToken->Translate(FoundPosition);

	for (const FIntPoint& Pos : Positions.Points)
	{
		FSpatialKeyedEntry Entry;
		Entry.Key.Key = Pos;
		Entry.Value = Key;
		OccupiedSlots.Insert(Entry.Key, Entry.Value);
	}
	return true;
}

void UInventorySpatialGridExtension::RemoveItemFromGrid(const FEntryKey& Key)
{
	TArray<FSpatialEntryKey> PositionsToRemove;

	for (const FSpatialKeyedEntry& Entry : OccupiedSlots.GetEntries())
	{
		if (Entry.Value == Key)
		{
			PositionsToRemove.Add(Entry.Key);
		}
	}

	for (const FSpatialEntryKey& Pos : PositionsToRemove)
	{
		OccupiedSlots.Remove(Pos);
	}
}

void UInventorySpatialGridExtension::PostInitProperties()
{
	Super::PostInitProperties();

	OccupiedSlots.ChangeListener = this;
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

EEventExtensionResponse UInventorySpatialGridExtension::AllowsAddition(const UFaerieItemContainerBase* Container,
                                                                       FFaerieItemStackView Stack)
{
	Super::AllowsAddition(Container, Stack);
	if (!CanAddItemToGrid(Stack.Item->GetToken<UFaerieShapeToken>()))
		return EEventExtensionResponse::Disallowed;

	return EEventExtensionResponse::Allowed;
}

inline void UInventorySpatialGridExtension::PostAddition(const UFaerieItemContainerBase* Container,
                                                         const Faerie::Inventory::FEventLog& Event)
{
	Super::PostAddition(Container, Event);
	FIntPoint NewPos = FIntPoint();
	AddItemToGrid(Event.EntryTouched,
	              Cast<UFaerieShapeToken>(
		              Container->Proxy(Event.EntryTouched).GetItemObject()->GetToken<UFaerieShapeToken>()),
	              NewPos);
}

inline void UInventorySpatialGridExtension::PostRemoval(const UFaerieItemContainerBase* Container,
                                                        const Faerie::Inventory::FEventLog& Event)
{
	Super::PostRemoval(Container, Event);
	RemoveItemFromGrid(Event.EntryTouched);
}
