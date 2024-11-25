// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "Extensions/InventoryGridExtensionBase.h"
#include "Net/UnrealNetwork.h"

void UInventoryGridExtensionBase::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams SharedParams;
	SharedParams.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, GridContent, SharedParams);
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, GridSize, SharedParams);
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, InitializedContainer, SharedParams);
}

void UInventoryGridExtensionBase::PostInitProperties()
{
	Super::PostInitProperties();
	GridContent.ChangeListener = this;
}

void UInventoryGridExtensionBase::DeinitializeExtension(const UFaerieItemContainerBase* Container)
{
	Super::DeinitializeExtension(Container);

	// Remove all entries for this container on shutdown
	// @todo its only okay to reset these because we don't suppose multi-container! revisit later
	OccupiedCells.Reset();
	GridContent.Items.Reset();
	InitializedContainer = nullptr;
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, InitializedContainer, this);
}

int32 UInventoryGridExtensionBase::Ravel(const FIntPoint& Point) const
{
	return Point.Y * GridSize.X + Point.X;
}

FIntPoint UInventoryGridExtensionBase::Unravel(const int32 Index) const
{
	const int32 X = Index % GridSize.X;
	const int32 Y = Index / GridSize.X;
	return FIntPoint{ X, Y };
}

void UInventoryGridExtensionBase::OnRep_GridSize()
{
	GridSizeChangedNative.Broadcast(GridSize);
	GridSizeChangedDelegate.Broadcast(GridSize);
}

FFaerieGridPlacement UInventoryGridExtensionBase::GetStackPlacementData(const FInventoryKey& Key) const
{
	if (auto&& Placement = GridContent.Find(Key))
	{
		return *Placement;
	}

	return FFaerieGridPlacement();
}

void UInventoryGridExtensionBase::SetGridSize(const FIntPoint NewGridSize)
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