// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "SpatialTypes.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(SpatialTypes)

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

FIntPoint FFaerieGridShape::GetShapeCenter() const
{
	return GetSize() / 2;
}

FIntPoint FFaerieGridShape::GetShapeAverageCenter() const
{
	if (Points.IsEmpty())
	{
		return FIntPoint::ZeroValue;
	}

	// To offset for 0-indexed points, initialize Sum with Points Num.
	// This is equivalent to adding 1,1 to every point.
	FIntPoint Sum(Points.Num());
	for (const FIntPoint& Point : Points)
	{
		Sum += Point;
	}

	return Sum / Points.Num();
}

bool FFaerieGridShape::IsSymmetrical() const
{
	if (Points.IsEmpty())
	{
		return true;
	}

	//create shape copy to compare against
	FFaerieGridShape ShapeCopy = *this;
	ShapeCopy.RotateAroundInline(ShapeCopy.GetShapeCenter());
	ShapeCopy.NormalizeInline();
	// Compare the shapes
	return ShapeCopy == *this;
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

void FFaerieGridShape::RotateInline(const ESpatialItemRotation Rotation)
{
	switch (Rotation)
	{
	case ESpatialItemRotation::Ninety:
		*this = RotateAngle(90.f);
		break;
	case ESpatialItemRotation::One_Eighty:
		*this = RotateAngle(180.f);
		break;
	case ESpatialItemRotation::Two_Seventy:
		*this = RotateAngle(270.f);
		break;
	case ESpatialItemRotation::None:
	case ESpatialItemRotation::MAX:
	default:
		break;
	}
}

FFaerieGridShape FFaerieGridShape::Rotate(const ESpatialItemRotation Rotation) const
{
	switch (Rotation)
	{
	case ESpatialItemRotation::Ninety:
		return RotateAngle(90.f);
	case ESpatialItemRotation::One_Eighty:
		return RotateAngle(180.f);
	case ESpatialItemRotation::Two_Seventy:
		return RotateAngle(270.f);
	case ESpatialItemRotation::None:
	case ESpatialItemRotation::MAX:
	default:
		return *this;
	}
}

void FFaerieGridShape::RotateAroundInline(const FIntPoint& PivotPoint)
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

FFaerieGridShape FFaerieGridShape::RotateAround(const FIntPoint& PivotPoint) const
{
	FFaerieGridShape NewShape = *this;
	NewShape.RotateAroundInline(PivotPoint);
	return NewShape;
}

FFaerieGridShape FFaerieGridShape::RotateAngle(const float AngleDegrees) const
{
	FFaerieGridShape NewShape = *this;

	// Get center point
	const FIntPoint Center = GetShapeCenter();

	// Convert angle to radians
	const float AngleRadians = FMath::DegreesToRadians(AngleDegrees);

	// Calculate sine and cosine
	const float CosTheta = FMath::Cos(AngleRadians);
	const float SinTheta = FMath::Sin(AngleRadians);

	for (FIntPoint& Point : NewShape.Points)
	{
		// Translate to origin
		const int32 TranslatedX = Point.X - Center.X;
		const int32 TranslatedY = Point.Y - Center.Y;

		// Rotate
		const float RotatedX = TranslatedX * CosTheta - TranslatedY * SinTheta;
		const float RotatedY = TranslatedX * SinTheta + TranslatedY * CosTheta;

		// Translate back and round
		Point.X = FMath::RoundToInt(RotatedX + Center.X);
		Point.Y = FMath::RoundToInt(RotatedY + Center.Y);
	}

	return NewShape;
}

void FFaerieGridShape::RotateAroundCenterInline()
{
	if (Points.IsEmpty())
	{
		return;
	}

	// Use existing rotation logic with calculated center
	RotateAroundInline(GetShapeCenter());
}

FFaerieGridShape FFaerieGridShape::RotateAroundCenter() const
{
	FFaerieGridShape NewShape = *this;
	NewShape.RotateAroundCenterInline();
	return NewShape;
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