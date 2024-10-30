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
	static FFaerieGridShape MakeRect(int32 Height, int32 Width);

	void Translate(const FIntPoint& Position);

	void NormalizeShape();
};