// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FaerieItemToken.h"
#include "FaerieShapeToken.generated.h"

struct FSpatialContent;
/**
 * 
 */
UCLASS()
class FAERIEITEMDATA_API UFaerieShapeToken : public UFaerieItemToken
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Token | Shape")
    TArray<FIntPoint> ShapeCoords;

    bool FitsInGrid(const FIntPoint& GridSize, const FIntPoint& Position,
                                   const FSpatialContent& Occupied) const;

    TArray<FIntPoint> GetOccupiedPositions(const FIntPoint& Position) const;

    FIntPoint GetWhereCanFit(const FIntPoint& GridSize, const FSpatialContent& Occupied) const;
};