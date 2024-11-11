// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "UFaerieGridLibrary.h"
#include "SpatialTypes.h"

FFaerieGridShape UFaerieGridLibrary::RotateShape(const FFaerieGridShape& InShape, const ESpatialItemRotation Rotation)
{
	return InShape.Rotate(Rotation);
}