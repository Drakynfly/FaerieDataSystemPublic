// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "CapacityStructs.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(CapacityStructs)

FItemCapacity UCapacityStructsUtilities::WeightOfScaledComparison(const FItemCapacity& Original,
                                                               const FItemCapacity& Comparison)
{
	FItemCapacity Out = Original;
	Out.Weight = static_cast<int32>(Original.GetEfficientVolume() * Comparison.WeightOfSquareCentimeter());
	return Out;
}

FWeightAndVolume UCapacityStructsUtilities::ToWeightAndVolume_ItemCapacity(const FItemCapacity& ItemCapacity)
{
	return FWeightAndVolume(ItemCapacity.Weight, ItemCapacity.GetVolume());
}