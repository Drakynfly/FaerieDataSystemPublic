// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.


#include "UI/SpatialGridWrapper.h"

#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "Extensions/InventorySpatialGridExtension.h"
#include "UI/SpatialDragVisual.h"
#include "UI/SpatialGridCell.h"
#include "UI/SpatialGridOperation.h"

void USpatialGridWrapper::NativeConstruct()
{
	Super::NativeConstruct();
}

bool USpatialGridWrapper::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
                                       UDragDropOperation* InOperation)
{
	USpatialGridOperation* SpatialDropOperation = Cast<USpatialGridOperation>(InOperation);
	SpatialDropOperation->InventoryExtension;
	// Don't call super if we don't have an inventory to work with
	if (!SpatialDropOperation->InventoryExtension) return false;
	if (const TObjectPtr<USpatialGridCell> WidgetUnderCursor = Cast<USpatialGridCell>(
		FindChildUnderCursor(InDragDropEvent.GetScreenSpacePosition(), SpatialDropOperation->GridPanel)))
	{
		const UUniformGridSlot* GridSlot = Cast<UUniformGridSlot>(WidgetUnderCursor->Slot);
		SpatialDropOperation->DragTo.X = GridSlot->GetColumn();
		SpatialDropOperation->DragTo.Y = GridSlot->GetRow();
		SpatialDropOperation->TargetOffset = SpatialDropOperation->DragTo - SpatialDropOperation->DragFrom;
		if (SpatialDropOperation->InventoryExtension->MoveItem(SpatialDropOperation->TargetKey,
		                                                       SpatialDropOperation->DragFrom,
		                                                       SpatialDropOperation->DragTo))
		{
			return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
		}
	}
	return false;
}

void USpatialGridWrapper::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent,
                                               UDragDropOperation*& OutOperation)
{
	
	OutOperation = NewObject<USpatialGridOperation>();
	USpatialGridOperation* SpatialDropOperation = Cast<USpatialGridOperation>(OutOperation);
	SpatialDropOperation->GridPanel = GridPanel;
	if (const TObjectPtr<USpatialGridCell> WidgetUnderCursor = Cast<USpatialGridCell>(
		FindChildUnderCursor(InMouseEvent.GetScreenSpacePosition(), SpatialDropOperation->GridPanel)))
	{
		const UUniformGridSlot* GridSlot = Cast<UUniformGridSlot>(WidgetUnderCursor->Slot);
		SpatialDropOperation->DragFrom.X = GridSlot->GetColumn();
		SpatialDropOperation->DragFrom.Y = GridSlot->GetRow();
		SpatialDropOperation->TargetKey = WidgetUnderCursor->SlotKey;
		SpatialDropOperation->TargetWidgets = GetChildrenByKey(SpatialDropOperation->TargetKey,
		                                                       SpatialDropOperation->GridPanel);
		SpatialDropOperation->InventoryExtension = ItemStorage->GetExtension<UInventorySpatialGridExtension>();
		SpatialDropOperation->ItemStorage = ItemStorage;
		// Create the drag visual widget
		if (USpatialDragVisual* DragVisual = CreateWidget<USpatialDragVisual>(GetOwningPlayer(), DragWidgetClass))
		{
			DragVisual->WidgetRefs = SpatialDropOperation->TargetWidgets;
			DragVisual->InitializeVisual();
			SpatialDropOperation->DefaultDragVisual = DragVisual;
			SpatialDropOperation->Pivot = EDragPivot::MouseDown;
		}
	}
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);
}

FReply USpatialGridWrapper::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	// Find widget under cursor and set "From" Targets
	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

TObjectPtr<UWidget> USpatialGridWrapper::FindChildUnderCursor(const FVector2D& AbsoluteMousePosition,
                                                              const UUniformGridPanel* GridPanel)
{
	// If we have no Grid there are no children
	if (!GridPanel) return nullptr;

	for (int32 i = 0; i < GridPanel->GetChildrenCount(); ++i)
	{
		UWidget* Child = GridPanel->GetChildAt(i);
		if (!Child || !Child->GetCachedWidget().IsValid()) continue;

		// Bounds check using child's size
		if (const FGeometry& ChildGeometry = Child->GetCachedGeometry(); ChildGeometry.IsUnderLocation(
			AbsoluteMousePosition))
		{
			return Child;
		}
	}
	// No Child Found
	return nullptr;
}

TArray<TObjectPtr<UWidget>> USpatialGridWrapper::GetChildrenByKey(const FInventoryKey& TargetKey,
                                                                  const UUniformGridPanel* GridPanel)
{
	TArray<TObjectPtr<UWidget>> ChildrenWithKey;
	// If no key is provided we will obviously have no children so return empty array
	if (!TargetKey.IsValid()) return ChildrenWithKey;
	for (int i = 0; i < GridPanel->GetChildrenCount(); ++i)
	{
		UWidget* Child = GridPanel->GetChildAt(i);
		if (const USpatialGridCell* Cell = Cast<USpatialGridCell>(Child); Cell->SlotKey == TargetKey)
		{
			ChildrenWithKey.Add(Child);
		}
	}
	return ChildrenWithKey;
}
