// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.


#include "ActorClasses/FaerieSpatialInventoryClient.h"

#include "FaerieItemStorage.h"
#include "Extensions/InventorySpatialGridExtension.h"

UFaerieSpatialInventoryClient::UFaerieSpatialInventoryClient()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void FFaerieClientAction_RequestMoveItemBetweenSpatialSlots::Server_Execute(const UFaerieInventoryClient* Client) const
{
	FFaerieClientActionBase::Server_Execute(Client);
}

void FFaerieClientAction_RequestRotateSpatialEntry::Server_Execute(const UFaerieInventoryClient* Client) const
{
	auto* SpatialExtension = Storage->GetExtension<UInventorySpatialGridExtension>();
	SpatialExtension->RotateItem(Key,Loc);
}

