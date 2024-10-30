// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "SpatialStructs.h"

FFaerieGridShape FFaerieGridShape::MakeRect(const int32 Height, const int32 Width)
{
	FFaerieGridShape OutShape;
	OutShape.Points.Reserve(Height * Width);
	for (int32 x = 0; x < Height; ++x)
	{
		for (int32 y = 0; y < Width; ++y)
		{
			OutShape.Points.Add(FIntPoint(x, y));
		}
	}
	return OutShape;
}