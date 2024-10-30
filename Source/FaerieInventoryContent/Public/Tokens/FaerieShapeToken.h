// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "FaerieItemToken.h"
#include "SpatialStructs.h"
#include "FaerieShapeToken.generated.h"

struct FSpatialContent;

/**
 * Adds a GridShape to an item. Used with Faerie Spatial Grid Extension.
 */
UCLASS(meta = (DisplayName = "Token - Shape"))
class FAERIEINVENTORYCONTENT_API UFaerieShapeToken : public UFaerieItemToken
{
    GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    bool FitsInGrid(const FIntPoint& GridSize, const FIntPoint& Position,
                                   const FSpatialContent& Occupied) const;

    FFaerieGridShape Translate(const FIntPoint& Position) const;

    TOptional<FIntPoint> GetFirstEmptyLocation(const FIntPoint& GridSize, const FSpatialContent& Occupied) const;

	static FFaerieGridShape NormalizeShape(const FFaerieGridShape& InputShape);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Replicated, meta = (ShowOnlyInnerProperties))
	FFaerieGridShape Shape;
};