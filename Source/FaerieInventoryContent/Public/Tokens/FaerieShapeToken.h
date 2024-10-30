// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "FaerieItemToken.h"
#include "FaerieShapeToken.generated.h"

struct FFaerieGridShape;
struct FSpatialContent;
/**
 * 
 */
UCLASS()
class FAERIEINVENTORYCONTENT_API UFaerieShapeToken : public UFaerieItemToken
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Token | Shape")
    TArray<FIntPoint> ShapeCoords;

    bool FitsInGrid(const FIntPoint& GridSize, const FIntPoint& Position,
                                   const FSpatialContent& Occupied) const;

    FFaerieGridShape Translate(const FIntPoint& Position) const;

    FIntPoint GetFirstEmptyLocation(const FIntPoint& GridSize, const FSpatialContent& Occupied) const;
};