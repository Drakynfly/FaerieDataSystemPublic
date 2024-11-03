// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InventoryDataStructs.h"
#include "Blueprint/DragDropOperation.h"
#include "Extensions/InventorySpatialGridExtension.h"
#include "SpatialGridOperation.generated.h"

class UUniformGridPanel;
/**
 * 
 */
UCLASS()
class FAERIEINVENTORYCONTENT_API USpatialGridOperation : public UDragDropOperation
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Grid")
	TObjectPtr<UUniformGridPanel> GridPanel;

	UPROPERTY(BlueprintReadOnly, Category = "Grid | Dragging Data", meta = (ExposeOnSpawn=true, InstanceEditable=true))
	TArray<TObjectPtr<UWidget>> TargetWidgets;
	
	UPROPERTY(BlueprintReadOnly, Category = "Grid | Dragging Data", meta = (ExposeOnSpawn=true, InstanceEditable=true))
	FIntPoint TargetOffset;

	UPROPERTY(BlueprintReadOnly, Category = "Grid | Dragging Data", meta = (ExposeOnSpawn=true, InstanceEditable=true))
	FIntPoint DragFrom;

	UPROPERTY(BlueprintReadOnly, Category = "Grid | Dragging Data", meta = (ExposeOnSpawn=true, InstanceEditable=true))
	FIntPoint DragTo;

	UPROPERTY(BlueprintReadOnly, Category = "Grid | Dragging Data", meta = (ExposeOnSpawn=true, InstanceEditable=true))
	FInventoryKey TargetKey;

	UPROPERTY(BlueprintReadOnly, Category = "Grid | Dragging Data", meta = (ExposeOnSpawn=true, InstanceEditable=true))
	FVector2D DragOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn=true, InstanceEditable=true))
	TObjectPtr<UInventorySpatialGridExtension> InventoryExtension;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn=true, InstanceEditable=true))
	TObjectPtr<UFaerieItemStorage> ItemStorage;
	
};
