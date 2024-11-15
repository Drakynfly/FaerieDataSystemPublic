// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "FaerieGridLibrary.h"
#include "SpatialTypes.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FaerieGridLibrary)

FFaerieGridShape UFaerieGridLibrary::RotateShape(const FFaerieGridShape& InShape, const ESpatialItemRotation Rotation)
{
	return InShape.Rotate(Rotation);
}