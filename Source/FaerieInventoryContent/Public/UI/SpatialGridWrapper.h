// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "FaerieItemStorage.h"
#include "InventoryDataStructs.h"
#include "Blueprint/UserWidget.h"
#include "SpatialGridWrapper.generated.h"

class UUniformGridPanel;
/**
 * 
 */
UCLASS()
class FAERIEINVENTORYCONTENT_API USpatialGridWrapper : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeConstruct() override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FIntPoint GridSize;
	
protected:
	TObjectPtr<UWidget> FindChildUnderCursor(const FVector2D& AbsoluteMousePosition) const;
	TArray<TObjectPtr<UWidget>> GetChildrenByKey(const FInventoryKey& TargetKey) const;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Grid")
	TObjectPtr<UUniformGridPanel> GridPanel;

	UPROPERTY(BlueprintReadOnly, Category = "Grid | Dragging Data")
	TArray<TObjectPtr<UWidget>> TargetWidgets;
	
	UPROPERTY(BlueprintReadOnly, Category = "Grid | Dragging Data")
	FIntPoint TargetOffset;

	UPROPERTY(BlueprintReadOnly, Category = "Grid | Dragging Data")
	FIntPoint DragFrom;

	UPROPERTY(BlueprintReadOnly, Category = "Grid | Dragging Data")
	FIntPoint DragTo;

	FInventoryKey CurrentKey;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UFaerieItemStorage> ItemStorage;
};
