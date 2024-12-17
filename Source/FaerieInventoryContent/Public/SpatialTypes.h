// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "FaerieGridEnums.h"
#include "SpatialTypes.generated.h"

struct FBitMatrix;
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
	FIntPoint GetIndexedShapeCenter() const;
	FIntPoint GetShapeAverageCenter() const;
	bool IsSymmetrical() const;
	template<typename T>
	T ToMatrix() const;
	template<typename T>
	TArray<FIntPoint> MatrixToPoints(const T& Matrix, FIntPoint Origin);
	void TransposeMatrix(FBitMatrix& Matrix) const;
	void ReverseMatrix(FBitMatrix& Matrix) const;
	template<typename T>
	T RotateMatrixClockwise(const T& Matrix, ESpatialItemRotation Rotation = ESpatialItemRotation::None) const;
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
	FInt32Rect GetBounds() const;
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
	FInt32Rect GetBounds() const;
	FIntPoint GetShapeCenter() const;
	FIntPoint GetShapeAverageCenter() const;
	bool IsSymmetrical() const;
	bool Contains(const FIntPoint& Position) const;
	bool Overlaps(const FFaerieGridShapeConstView& Other) const;

	FFaerieGridShape Copy() const;

	friend bool operator==(const FFaerieGridShapeConstView& Lhs, const FFaerieGridShapeConstView& Rhs);
	friend bool operator!=(const FFaerieGridShapeConstView& Lhs, const FFaerieGridShapeConstView& Rhs) { return !(Lhs == Rhs); }
};

// Add this to your class declaration
struct FBitMatrix
{
	TArray<uint32> Data;
	int32 Width;
	int32 Height;

	void Init(int32 InWidth, int32 InHeight)
	{
		Width = InWidth;
		Height = InHeight;
		// Calculate how many uint32s we need to store all bits
		int32 NumInts = (Width * Height + 31) / 32;
		Data.SetNum(NumInts, false);
	}

	void Set(int32 X, int32 Y, bool Value)
	{
		int32 Index = Y * Width + X;
		int32 IntIndex = Index / 32;
		int32 BitIndex = Index % 32;
        
		if (Value)
		{
			Data[IntIndex] |= (1u << BitIndex);
		}
		else
		{
			Data[IntIndex] &= ~(1u << BitIndex);
		}
	}

	bool Get(int32 X, int32 Y) const
	{
		int32 Index = Y * Width + X;
		int32 IntIndex = Index / 32;
		int32 BitIndex = Index % 32;
		return (Data[IntIndex] & (1u << BitIndex)) != 0;
	}
};