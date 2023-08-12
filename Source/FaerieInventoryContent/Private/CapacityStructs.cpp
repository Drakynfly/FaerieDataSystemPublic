// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "CapacityStructs.h"

FItemCapacity UCapacityStructsUtilities::WeightOfScaledComparison(const FItemCapacity& Original,
                                                               const FItemCapacity& Comparison)
{
	FItemCapacity Out = Original;
	Out.Weight = Original.GetEfficientVolume() * Comparison.WeightOfSquareCentimeter();
	return Out;
}

FWeightAndVolume UCapacityStructsUtilities::ToWeightAndVolume_ItemCapacity(const FItemCapacity& ItemCapacity)
{
	return FWeightAndVolume(ItemCapacity.Weight, ItemCapacity.GetVolume());
}