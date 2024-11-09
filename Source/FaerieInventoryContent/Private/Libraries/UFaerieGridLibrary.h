// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "UFaerieGridLibrary.generated.h"

enum class ESpatialItemRotation : uint8;
struct FFaerieGridShape;
/**
 * 
 */
UCLASS()
class UFaerieGridLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="Shape Manipulation")
	static FFaerieGridShape RotateShape(FFaerieGridShape InShape, const ESpatialItemRotation Rotation);
};
