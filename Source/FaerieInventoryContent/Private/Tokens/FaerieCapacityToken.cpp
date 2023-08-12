// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "Tokens/FaerieCapacityToken.h"
#include "Extensions/InventoryCapacityExtension.h"
#include "Net/UnrealNetwork.h"

void UFaerieCapacityToken::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams Params;
	Params.bIsPushBased = true;
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, Capacity, Params);
}

int32 UFaerieCapacityToken::GetWeightOfStack(const FInventoryStack Stack) const
{
	return Capacity.Weight * Stack.GetAmount();
}

int32 UFaerieCapacityToken::GetVolumeOfStack(const FInventoryStack Stack) const
{
	const int32 Volume = Capacity.GetVolume();
	return Volume + (Volume * (Stack.GetAmount() - 1) * Capacity.Efficiency);
}

FWeightAndVolume UFaerieCapacityToken::GetWeightAndVolumeOfStack(const FInventoryStack Stack) const
{
	return FWeightAndVolume(GetWeightOfStack(Stack), GetVolumeOfStack(Stack));
}

FWeightAndVolume UFaerieCapacityToken::GetWeightAndVolumeOfStackForRemoval(const FInventoryStack Current,
																		   const FInventoryStack Removal) const
{
	if (Removal <= 0) return FWeightAndVolume();

	FWeightAndVolume Out;

	if (Removal >= Current)
	{
		Out.GramWeight = GetWeightOfStack(Current);
		Out.Volume = GetVolumeOfStack(Current);
	}
	else
	{
		Out.GramWeight = Capacity.Weight * Removal.GetAmount();
		Out.Volume = Capacity.GetVolume() * Removal.GetAmount() * Capacity.Efficiency;
	}

	return Out;
}
