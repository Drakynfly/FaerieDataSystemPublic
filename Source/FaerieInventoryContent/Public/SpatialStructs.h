// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "SpatialStructs.generated.h"

/*
 * A shape composed of 2D points.
 */
USTRUCT(BlueprintType)
struct FAERIEINVENTORYCONTENT_API FFaerieGridShape
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Grid")
	TArray<FIntPoint> Points;

	// Make a rectangular shape
	static FFaerieGridShape MakeSquare(int32 Size);
	static FFaerieGridShape MakeRect(int32 Height, int32 Width);

	FIntPoint GetSize() const;
	FIntPoint GetShapeCenter();
	bool CanRotate() const;

	void TranslateInline(const FIntPoint& Position);
	[[nodiscard]] FFaerieGridShape Translate(const FIntPoint& Position) const;

	/* 90 degree rotation */
	void RotateInline(const FIntPoint& PivotPoint);
	[[nodiscard]] FFaerieGridShape Rotate(const FIntPoint& PivotPoint) const;

	void RotateAroundCenterInline();
	[[nodiscard]] FFaerieGridShape RotateAroundCenter() const;

	void RotateAboutAngle(float AngleDegrees);

	void NormalizeInline();
	[[nodiscard]] FFaerieGridShape Normalize() const;

	friend bool operator==(const FFaerieGridShape& Lhs, const FFaerieGridShape& Rhs);
	friend bool operator!=(const FFaerieGridShape& Lhs, const FFaerieGridShape& Rhs) { return !(Lhs == Rhs); }
};