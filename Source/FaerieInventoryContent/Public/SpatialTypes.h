// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "FaerieGridEnums.h"
#include "SpatialTypes.generated.h"

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
	FIntRect GetBounds() const;
	FIntPoint GetShapeCenter() const;
	FIntPoint GetShapeAverageCenter() const;
	bool IsSymmetrical() const;
	bool Contains(const FIntPoint& Position) const;
	[[nodiscard]] bool Overlaps(const FFaerieGridShape& Other) const;

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

/*
 * A view of a FFaerieGridShape.
 */
struct FAERIEINVENTORYCONTENT_API FFaerieGridShapeView
{
	TArrayView<FIntPoint> Points;

	FFaerieGridShapeView(FFaerieGridShape& Shape)
	  : Points(Shape.Points) {}

	FIntPoint GetSize() const;
	FIntRect GetBounds() const;
	FIntPoint GetShapeCenter() const;
	FIntPoint GetShapeAverageCenter() const;
	bool IsSymmetrical() const;
	bool Contains(const FIntPoint& Position) const;
	bool Overlaps(const FFaerieGridShapeView& Other) const;

	void TranslateInline(const FIntPoint& Position);
	[[nodiscard]] FFaerieGridShapeView Translate(const FIntPoint& Position) const;

	void RotateInline(ESpatialItemRotation Rotation);
	[[nodiscard]] FFaerieGridShapeView Rotate(ESpatialItemRotation Rotation) const;

	void RotateAroundInline_90(const FIntPoint& PivotPoint);
	[[nodiscard]] FFaerieGridShapeView RotateAround_90(const FIntPoint& PivotPoint) const;
	void RotateAroundInline_180(const FIntPoint& PivotPoint);
	[[nodiscard]] FFaerieGridShapeView RotateAround_180(const FIntPoint& PivotPoint) const;
	void RotateAroundInline_270(const FIntPoint& PivotPoint);
	[[nodiscard]] FFaerieGridShapeView RotateAround_270(const FIntPoint& PivotPoint) const;

	void RotateAroundCenterInline();
	[[nodiscard]] FFaerieGridShapeView RotateAroundCenter() const;

	void NormalizeInline();
	[[nodiscard]] FFaerieGridShapeView Normalize() const;

	friend bool operator==(const FFaerieGridShapeView& Lhs, const FFaerieGridShapeView& Rhs);
	friend bool operator!=(const FFaerieGridShapeView& Lhs, const FFaerieGridShapeView& Rhs) { return !(Lhs == Rhs); }
};

/*
 * A const view of a FFaerieGridShape.
 */
struct FAERIEINVENTORYCONTENT_API FFaerieGridShapeConstView
{
	TConstArrayView<FIntPoint> Points;

	FFaerieGridShapeConstView() = default;

	FFaerieGridShapeConstView(const FFaerieGridShape& Shape)
	  : Points(Shape.Points) {}

	FIntPoint GetSize() const;
	FIntRect GetBounds() const;
	FIntPoint GetShapeCenter() const;
	FIntPoint GetShapeAverageCenter() const;
	bool IsSymmetrical() const;
	bool Contains(const FIntPoint& Position) const;
	bool Overlaps(const FFaerieGridShapeConstView& Other) const;

	FFaerieGridShape Copy() const;

	friend bool operator==(const FFaerieGridShapeConstView& Lhs, const FFaerieGridShapeConstView& Rhs);
	friend bool operator!=(const FFaerieGridShapeConstView& Lhs, const FFaerieGridShapeConstView& Rhs) { return !(Lhs == Rhs); }
};