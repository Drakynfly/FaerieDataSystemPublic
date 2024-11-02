// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.


#include "UI/SpatialGridWrapper.h"

#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "Extensions/InventorySpatialGridExtension.h"
#include "UI/SpatialGridCell.h"

void USpatialGridWrapper::NativeConstruct()
{
	Super::NativeConstruct();
}

bool USpatialGridWrapper::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
                                       UDragDropOperation* InOperation)
{
	// Don't call super if we don't have an inventory to work with
	if (!ItemStorage) return false;
	if (const TObjectPtr<USpatialGridCell> WidgetUnderCursor = Cast<USpatialGridCell>(
		FindChildUnderCursor(InDragDropEvent.GetScreenSpacePosition())))
	{
		const UUniformGridSlot* GridSlot = Cast<UUniformGridSlot>(WidgetUnderCursor->Slot);
		DragTo.X = GridSlot->GetColumn();
		DragTo.Y = GridSlot->GetRow();
		TargetOffset = DragTo - DragFrom;
		if (ItemStorage->GetExtension<UInventorySpatialGridExtension>()->MoveItem(CurrentKey, DragFrom, DragTo))
		{
			return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
		}
	}
	return false;
}

FReply USpatialGridWrapper::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	// Find widget under cursor and set "From" Targets

	if (const TObjectPtr<USpatialGridCell> WidgetUnderCursor = Cast<USpatialGridCell>(
		FindChildUnderCursor(InMouseEvent.GetScreenSpacePosition())))
	{
		const UUniformGridSlot* GridSlot = Cast<UUniformGridSlot>(WidgetUnderCursor->Slot);
		DragFrom.X = GridSlot->GetColumn();
		DragFrom.Y = GridSlot->GetRow();
		CurrentKey = WidgetUnderCursor->SlotKey;
		TargetWidgets = GetChildrenByKey(CurrentKey);
	}

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

TObjectPtr<UWidget> USpatialGridWrapper::FindChildUnderCursor(const FVector2D& AbsoluteMousePosition) const
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

TArray<TObjectPtr<UWidget>> USpatialGridWrapper::GetChildrenByKey(const FInventoryKey& TargetKey) const
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
