// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.


#include "UGridHelperLibrary.h"

#include "SpatialStructs.h"

FFaerieGridShape UGridHelperLibrary::RotateShape(FFaerieGridShape InShape, const ESpatialItemRotation Rotation)
{
	InShape.RotateAboutAngle(static_cast<float>(Rotation) * 90.f);
	return InShape;
}
