// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "FaerieItemStorage.h"
#include "InventoryDataStructs.h"
#include "Blueprint/UserWidget.h"
#include "Extensions/InventorySpatialGridExtension.h"
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
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UFaerieItemStorage> ItemStorage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UUniformGridPanel> GridPanel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UUserWidget> DragWidgetClass;
	
protected:
	static TObjectPtr<UWidget> FindChildUnderCursor(const FVector2D& AbsoluteMousePosition, const UUniformGridPanel* GridPanel);
	static TArray<TObjectPtr<UWidget>> GetChildrenByKey(const FInventoryKey& TargetKey, const UUniformGridPanel* GridPanel);
};
