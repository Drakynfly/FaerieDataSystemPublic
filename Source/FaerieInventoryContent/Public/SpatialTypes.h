// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "SpatialTypes.generated.h"

/* Degrees to rotate a shape by */
UENUM(BlueprintType)
enum class ESpatialItemRotation : uint8
{
	None = 0,
	Ninety = 1,
	One_Eighty = 2,
	Two_Seventy = 3,
	MAX UMETA(Hidden)
};
ENUM_RANGE_BY_COUNT(ESpatialItemRotation, ESpatialItemRotation::MAX)

FORCEINLINE ESpatialItemRotation GetNextRotation(const ESpatialItemRotation CurrentRotation)
{
	switch (CurrentRotation)
	{
	case ESpatialItemRotation::None:
		return ESpatialItemRotation::Ninety;
	case ESpatialItemRotation::Ninety:
		return ESpatialItemRotation::One_Eighty;
	case ESpatialItemRotation::One_Eighty:
		return ESpatialItemRotation::Two_Seventy;
	case ESpatialItemRotation::Two_Seventy:
		return ESpatialItemRotation::None;
	default:
		return ESpatialItemRotation::None;
	}
}

FORCEINLINE ESpatialItemRotation GetPreviousRotation(const ESpatialItemRotation CurrentRotation)
{
	switch (CurrentRotation)
	{
	case ESpatialItemRotation::None:
		return ESpatialItemRotation::Two_Seventy;
	case ESpatialItemRotation::Ninety:
		return ESpatialItemRotation::None;
	case ESpatialItemRotation::One_Eighty:
		return ESpatialItemRotation::Ninety;
	case ESpatialItemRotation::Two_Seventy:
		return ESpatialItemRotation::One_Eighty;
	default:
		return ESpatialItemRotation::None;
	}
}

/*
 * A shape composed of 2D points.
 */
USTRUCT(BlueprintType)
struct FAERIEINVENTORYCONTENT_API FFaerieGridShape
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "FaerieGridShape")
	TArray<FIntPoint> Points;

	// Make a rectangular shape
	static FFaerieGridShape MakeSquare(int32 Size);
	static FFaerieGridShape MakeRect(int32 Height, int32 Width);

	FIntPoint GetSize() const;
	FInt32Rect GetBounds() const;
	FIntPoint GetShapeCenter() const;
	FIntPoint GetShapeAverageCenter() const;
	bool IsSymmetrical() const;

	void TranslateInline(const FIntPoint& Position);
	[[nodiscard]] FFaerieGridShape Translate(const FIntPoint& Position) const;

	void RotateInline(ESpatialItemRotation Rotation);
	[[nodiscard]] FFaerieGridShape Rotate(ESpatialItemRotation Rotation) const;

	void RotateAroundInline_90(const FIntPoint& PivotPoint);
	[[nodiscard]] FFaerieGridShape RotateAround_90(const FIntPoint& PivotPoint) const;
	void RotateAroundInline_180(const FIntPoint& PivotPoint);
	[[nodiscard]] FFaerieGridShape RotateAround_180(const FIntPoint& PivotPoint) const;
	void RotateAroundInline_270(const FIntPoint& PivotPoint);
	[[nodiscard]] FFaerieGridShape RotateAround_270(const FIntPoint& PivotPoint) const;

	void RotateAroundCenterInline();
	[[nodiscard]] FFaerieGridShape RotateAroundCenter() const;

	void NormalizeInline();
	[[nodiscard]] FFaerieGridShape Normalize() const;

	friend bool operator==(const FFaerieGridShape& Lhs, const FFaerieGridShape& Rhs);
	friend bool operator!=(const FFaerieGridShape& Lhs, const FFaerieGridShape& Rhs) { return !(Lhs == Rhs); }

private:
	// Internal rotation util
	[[nodiscard]] FFaerieGridShape RotateAngle(float AngleDegrees) const;
};