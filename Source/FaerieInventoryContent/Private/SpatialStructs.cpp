// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "SpatialStructs.h"

FFaerieGridShape FFaerieGridShape::MakeSquare(const int32 Size)
{
	return MakeRect(Size, Size);
}

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

FIntPoint FFaerieGridShape::GetSize() const
{
	FFaerieGridShape Normal = Normalize();
	FIntPoint Size = FIntPoint::ZeroValue;
	for (auto&& Point : Normal.Points)
	{
		Size.X = FMath::Max(Size.X, Point.X);
		Size.Y = FMath::Max(Size.Y, Point.Y);
	}
	return Size;
}

bool FFaerieGridShape::CanRotate() const
{
	const FIntPoint Size = GetSize();

	// If our dimensions are dissimilar, we can be rotated
	if (Size.X != Size.Y) return true;

	// If we are a square of our dimension, we cannot rotate
	return MakeSquare(Size.X) != *this;

	// @todo there are other shapes that cannot rotate! any shape that is radially symmetrical at 90 degree angles is un-rotatable.
}

void FFaerieGridShape::TranslateInline(const FIntPoint& Position)
{
	for (FIntPoint& Coord : Points)
	{
		Coord += Position;
	}
}

FFaerieGridShape FFaerieGridShape::Translate(const FIntPoint& Position) const
{
	FFaerieGridShape OutShape = *this;
	OutShape.TranslateInline(Position);
	return OutShape;
}

void FFaerieGridShape::RotateInline(const FIntPoint& PivotPoint)
{
	for (FIntPoint& Point : Points)
	{
		// Rebase to pivot
		FIntPoint Relative = Point - PivotPoint;

		// Trade places
		Swap(Relative.X, Relative.Y);

		// Flip Y
		Relative.Y *= -1;

		// Remove rebase
		Point = Relative + PivotPoint;
	}
}

FFaerieGridShape FFaerieGridShape::Rotate(const FIntPoint& PivotPoint) const
{
	FFaerieGridShape NewShape = *this;
	NewShape.RotateInline(PivotPoint);
	return NewShape;
}

void FFaerieGridShape::RotateAroundCenterInline()
{
	if (Points.IsEmpty())
	{
		return;
	}

	// Calculate the center point
	FVector2D Center = GetShapeCenter();

	// Convert center to FIntPoint, rounding to nearest integer
	FIntPoint PivotPoint(
		FMath::RoundToInt(Center.X),
		FMath::RoundToInt(Center.Y)
	);

	// Use existing rotation logic with calculated center
	RotateInline(PivotPoint);
}

FFaerieGridShape FFaerieGridShape::RotateAroundCenter() const
{
	FFaerieGridShape NewShape = *this;
	NewShape.RotateAroundCenterInline();
	return NewShape;
}

void FFaerieGridShape::RotateAboutAngle(float AngleDegrees)
{
	// Get center point
	FIntPoint Center = GetShapeCenter();
    
	// Convert angle to radians
	float AngleRadians = FMath::DegreesToRadians(AngleDegrees);

	// Calculate sine and cosine
	float CosTheta = FMath::Cos(AngleRadians);
	float SinTheta = FMath::Sin(AngleRadians);

	for(FIntPoint& Point : Points)
	{
		// Translate to origin
		int32 TranslatedX = Point.X - Center.X;
		int32 TranslatedY = Point.Y - Center.Y;
        
		// Rotate
		float RotatedX = TranslatedX * CosTheta - TranslatedY * SinTheta;
		float RotatedY = TranslatedX * SinTheta + TranslatedY * CosTheta;
        
		// Translate back and round
		Point.X = FMath::RoundToInt(RotatedX + Center.X);
		Point.Y = FMath::RoundToInt(RotatedY + Center.Y);
	}
}

void FFaerieGridShape::NormalizeInline()
{
	if (Points.IsEmpty())
	{
		return;
	}

	FIntPoint Min(TNumericLimits<int32>::Max());

	for (const FIntPoint& Point : Points)
	{
		Min.X = FMath::Min(Min.X, Point.X);
		Min.Y = FMath::Min(Min.Y, Point.Y);
	}

	for (FIntPoint& Point : Points)
	{
		Point -= Min;
	}
}

FFaerieGridShape FFaerieGridShape::Normalize() const
{
	FFaerieGridShape NewShape = *this;
	NewShape.NormalizeInline();
	return NewShape;
}

bool operator==(const FFaerieGridShape& Lhs, const FFaerieGridShape& Rhs)
{
	// Mismatching point numbers; auto-fail
	if (Lhs.Points.Num() != Rhs.Points.Num()) return false;

	// @todo points have to be found individually, instead of just comparing the arrays, because the same points are not guaranteed to be in the same order.
	// to fix this, Points would have to be sorted. until then, this is really slow!!
	for (auto&& Point : Lhs.Points)
	{
		if (!Rhs.Points.Contains(Point)) return false;
	}

	return true;
}

FIntPoint FFaerieGridShape::GetShapeCenter()
{
    if (Points.IsEmpty())
    {
        return FIntPoint::ZeroValue;
    }

    float SumX = 0.0f;
    float SumY = 0.0f;
    for (const FIntPoint& Point : Points)
    {
        SumX += Point.X;
        SumY += Point.Y;
    }

    // Calculate floating point average and round properly
    float AvgX = SumX / Points.Num();
    float AvgY = SumY / Points.Num();

    // Use FMath::RoundToInt for proper rounding
    return FIntPoint(FMath::RoundToInt(AvgX), FMath::RoundToInt(AvgY));
}
