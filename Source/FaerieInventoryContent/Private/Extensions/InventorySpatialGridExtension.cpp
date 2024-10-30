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
	//Todo
}

void FSpatialContent::PostEntryReplicatedAdd(const FSpatialKeyedEntry& Entry)
{
	ChangeListener->PostEntryReplicatedAdd(Entry);
	Sort();
}

void FSpatialContent::PostEntryReplicatedChange(const FSpatialKeyedEntry& Entry) const
{
	//Todo
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
}

void UInventorySpatialGridExtension::PostInitProperties()
{
	Super::PostInitProperties();

	OccupiedSlots.ChangeListener = this;
}

EEventExtensionResponse UInventorySpatialGridExtension::AllowsAddition(const UFaerieItemContainerBase* Container, const FFaerieItemStackView Stack)
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

void UInventorySpatialGridExtension::PostEntryReplicatedAdd(const FSpatialKeyedEntry& Entry)
{
	SpatialEntryChangedDelegateNative.Broadcast(Entry.Value);
	SpatialEntryChangedDelegate.Broadcast(Entry.Value);
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

bool UInventorySpatialGridExtension::AddItemToGrid(const FEntryKey& Key, const UFaerieShapeToken* ShapeToken)
{
	if (!ShapeToken ||
		!Key.IsValid())
	{
		return false;
	}

	TOptional<FIntPoint> FoundPosition = ShapeToken->GetFirstEmptyLocation(GridSize, OccupiedSlots);
	if (!FoundPosition.IsSet()) return false;

	const FFaerieGridShape Positions = ShapeToken->Translate(FoundPosition.GetValue());
	for (const FIntPoint& Pos : Positions.Points)
	{
		const FSpatialEntryKey EntryKey { Pos };
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