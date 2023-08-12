// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "Tokens/FaerieItemStorageToken.h"
#include "FaerieItemStorage.h"
#include "Net/UnrealNetwork.h"

UFaerieItemStorageToken::UFaerieItemStorageToken()
{
	ItemStorage = CreateDefaultSubobject<UFaerieItemStorage>("ItemStorage");
}

void UFaerieItemStorageToken::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ThisClass, ItemStorage, COND_InitialOnly);
}

bool UFaerieItemStorageToken::IsMutable() const
{
	// Storage tokens always make their owner mutable, as should be obvious. If an item can container arbitrary content
	// then we have no way to determine its mutability state.
	return true;
}