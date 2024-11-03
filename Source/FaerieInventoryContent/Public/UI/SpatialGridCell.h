// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "InventoryDataStructs.h"
#include "Blueprint/UserWidget.h"
#include "SpatialGridCell.generated.h"

class USpatialGridWrapper;
/**
 * 
 */
UCLASS()
class FAERIEINVENTORYCONTENT_API USpatialGridCell : public UUserWidget
{
	GENERATED_BODY()
	friend USpatialGridWrapper;
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FInventoryKey SlotKey;
};
