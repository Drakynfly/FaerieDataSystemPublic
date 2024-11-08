// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "UGridHelperLibrary.generated.h"

enum class ESpatialItemRotation : uint8;
struct FFaerieGridShape;
/**
 * 
 */
UCLASS()
class FAERIEINVENTORYCONTENT_API UGridHelperLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable, Category="Shape Manipulation")
	static FFaerieGridShape RotateShape(FFaerieGridShape InShape, const ESpatialItemRotation Rotation);
};
