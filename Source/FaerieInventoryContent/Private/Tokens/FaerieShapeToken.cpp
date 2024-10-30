// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "Tokens/FaerieShapeToken.h"
#include "Extensions/InventorySpatialGridExtension.h"
#include "Net/UnrealNetwork.h"

void UFaerieShapeToken::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    FDoRepLifetimeParams Params;
    Params.bIsPushBased = true;
    DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, Shape, Params);
}

bool UFaerieShapeToken::FitsInGrid(const FIntPoint& GridSize, const FIntPoint& Position,
                                   const FSpatialContent& Occupied) const
{
    FFaerieGridShape NormalizedShape = NormalizeShape(Shape);
    for (const FIntPoint& Coord : NormalizedShape.Points)
    {
        const FIntPoint AbsolutePosition = Position + Coord;

        if (AbsolutePosition.X < 0 || AbsolutePosition.X >= GridSize.X ||
            AbsolutePosition.Y < 0 || AbsolutePosition.Y >= GridSize.Y)
        {
            return false;
        }

        for (const FSpatialKeyedEntry& Entry : Occupied.GetEntries())
        {
            if (Entry.Key.Key == AbsolutePosition)
            {
                return false;
            }
        }
    }
    return true;
}

FFaerieGridShape UFaerieShapeToken::Translate(const FIntPoint& Position) const
{
    FFaerieGridShape OccupiedPositions;
    FFaerieGridShape NormalizedShape = NormalizeShape(Shape);
    OccupiedPositions.Points.Reserve(NormalizedShape.Points.Num());
    for (const FIntPoint& Coord : NormalizedShape.Points)
    {
        OccupiedPositions.Points.Add(Position + Coord);
    }
    return OccupiedPositions;
}

TOptional<FIntPoint> UFaerieShapeToken::GetFirstEmptyLocation(const FIntPoint& GridSize, const FSpatialContent& Occupied) const
{
    FIntPoint TestPosition = FIntPoint::ZeroValue;
    for (; TestPosition.Y < GridSize.Y; TestPosition.Y++)
    {
        for (; TestPosition.X < GridSize.X; TestPosition.X++)
        {
            if (FitsInGrid(GridSize, TestPosition, Occupied))
            {
                return TestPosition;
            }
        }
    }
    return NullOpt;
}

FFaerieGridShape UFaerieShapeToken::NormalizeShape(const FFaerieGridShape& InputShape)
{
    if (InputShape.Points.IsEmpty())
    {
        return InputShape;
    }
    
    int32 MinX = InputShape.Points[0].X;
    int32 MinY = InputShape.Points[0].Y;
    
    for (const FIntPoint& Point : InputShape.Points)
    {
        MinX = FMath::Min(MinX, Point.X);
        MinY = FMath::Min(MinY, Point.Y);
    }
    
    FFaerieGridShape NormalizedShape;
    NormalizedShape.Points.Reserve(InputShape.Points.Num());
    
    for (const FIntPoint& Point : InputShape.Points)
    {
        NormalizedShape.Points.Add(FIntPoint(
            Point.X - MinX,
            Point.Y - MinY
        ));
    }
    
    return NormalizedShape;
}