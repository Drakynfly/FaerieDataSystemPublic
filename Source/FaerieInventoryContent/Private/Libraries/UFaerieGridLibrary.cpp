// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "UFaerieGridLibrary.h"
#include "SpatialTypes.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UFaerieGridLibrary)

FFaerieGridShape UFaerieGridLibrary::RotateShape(const FFaerieGridShape& InShape, const ESpatialItemRotation Rotation)
{
	return InShape.Rotate(Rotation);
}