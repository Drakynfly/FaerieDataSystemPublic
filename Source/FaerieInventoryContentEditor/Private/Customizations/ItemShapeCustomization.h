// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.
#pragma once

#include "Input/Reply.h"
#include "IPropertyTypeCustomization.h"

class SUniformGridPanel;
/**
 * 
 */
class FItemShapeCustomization : public IPropertyTypeCustomization
{
public:
	static TSharedRef<IPropertyTypeCustomization> MakeInstance();
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils) override;
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils) override;
	void UpdateGridPanel();
	

private:
	void OnGridSizeChanged(int32 NewValue, ETextCommit::Type CommitType);
	void OnCellClicked(FIntPoint CellCoord);
	bool IsCellSelected(FIntPoint CellCoord) const;

	IDetailChildrenBuilder* Builder = nullptr;
	IPropertyTypeCustomizationUtils* Utils = nullptr;
	TSharedPtr<IPropertyHandle> StructHandle;
	TSharedPtr<SUniformGridPanel> GridPanel;
	int32 GridSize = 10;
};
