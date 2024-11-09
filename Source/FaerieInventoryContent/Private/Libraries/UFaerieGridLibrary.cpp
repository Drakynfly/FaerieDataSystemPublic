// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.


#include "UFaerieGridLibrary.h"

#include "SpatialStructs.h"

FFaerieGridShape UFaerieGridLibrary::RotateShape(FFaerieGridShape InShape, const ESpatialItemRotation Rotation)
{
	InShape.RotateAboutAngle(static_cast<float>(Rotation) * 90.f);
	return InShape;
}
