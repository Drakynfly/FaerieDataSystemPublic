// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.


#include "ItemShapeCustomization.h"

#include "DetailWidgetRow.h"
#include "Widgets/Input/SSpinBox.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "IDetailChildrenBuilder.h"
#include "IPropertyUtilities.h"
#include "SpatialStructs.h"

#define LOCTEXT_NAMESPACE "ShapeGridCustomization"

TSharedRef<IPropertyTypeCustomization> FItemShapeCustomization::MakeInstance()
{
    return MakeShareable(new FItemShapeCustomization());
}

void FItemShapeCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
    StructHandle = PropertyHandle;
    
    HeaderRow.NameContent()
    [
        PropertyHandle->CreatePropertyNameWidget(FText().FromString("Item Shape Builder"))
    ];
}

void FItemShapeCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
    const float CellSize = 25.0f;
    
    GridPanel = SNew(SUniformGridPanel)
    .SlotPadding(FMargin(1));
    
    for (int32 Y = 0; Y < GridSize; Y++)
    {
        for (int32 X = 0; X < GridSize; X++)
        {
            const FIntPoint CellCoord(X, Y);
            GridPanel->AddSlot(X, Y)
            [
                SNew(SBox)
                .HeightOverride(CellSize)
                .WidthOverride(CellSize)
                [
                    SNew(SButton)
                    .OnClicked_Lambda([this, CellCoord]()
                    {
                        OnCellClicked(CellCoord);
                        return FReply::Handled();
                    })
                    .ButtonColorAndOpacity_Lambda([this, CellCoord]()
                    {
                        return IsCellSelected(CellCoord) ? FLinearColor::Blue : FLinearColor::White;
                    })
                    .ContentPadding(FMargin(0))
                    .DesiredSizeScale(FVector2D(1.0f, 1.0f))
                    .HAlign(HAlign_Fill)
                    .VAlign(VAlign_Fill)
                ]
            ];
        }
    }
    
    ChildBuilder.AddCustomRow(LOCTEXT("GridRow", "Grid"))
    .WholeRowContent()
    [
        SNew(SBox)
        .Padding(FMargin(0, 5))
        .HAlign(HAlign_Center)
        [
            GridPanel.ToSharedRef()
        ]
    ];

    Builder = &ChildBuilder;
    Utils = &CustomizationUtils;
}


void FItemShapeCustomization::UpdateGridPanel()
{
    if (!Builder) return;
    
    const float CellSize = 25.0f;
    
    GridPanel->ClearChildren();
    
    for (int32 Y = 0; Y < GridSize; Y++)
    {
        for (int32 X = 0; X < GridSize; X++)
        {
            const FIntPoint CellCoord(X, Y);
            GridPanel->AddSlot(X, Y)
            [
                SNew(SBox)
                .HeightOverride(CellSize)
                .WidthOverride(CellSize)
                [
                    SNew(SButton)
                    .OnClicked_Lambda([this, CellCoord]()
                    {
                        OnCellClicked(CellCoord);
                        return FReply::Handled();
                    })
                    .ButtonColorAndOpacity_Lambda([this, CellCoord]()
                    {
                        return IsCellSelected(CellCoord) ? FLinearColor::Blue : FLinearColor::White;
                    })
                    .ContentPadding(FMargin(0))
                    .DesiredSizeScale(FVector2D(1.0f, 1.0f))
                    .HAlign(HAlign_Fill)
                    .VAlign(VAlign_Fill)
                ]
            ];
        }
    }
}

void FItemShapeCustomization::OnGridSizeChanged(int32 NewValue, ETextCommit::Type CommitType)
{
    GridSize = NewValue;
    
    TArray<FIntPoint> CurrentPoints;
    void* StructPtr;
    StructHandle->GetValueData(StructPtr);
    FFaerieGridShape* ShapeGrid = static_cast<FFaerieGridShape*>(StructPtr);
    if (ShapeGrid)
    {
        CurrentPoints = ShapeGrid->Points;
        CurrentPoints.RemoveAll([this](const FIntPoint& Point) {
            return Point.X >= GridSize || Point.Y >= GridSize;
        });
        ShapeGrid->Points = CurrentPoints;
        StructHandle->NotifyPostChange(EPropertyChangeType::ArrayAdd);
    }

    UpdateGridPanel();
}

void FItemShapeCustomization::OnCellClicked(FIntPoint CellCoord)
{
    void* StructPtr;
    StructHandle->GetValueData(StructPtr);
    FFaerieGridShape* ShapeGrid = static_cast<FFaerieGridShape*>(StructPtr);
    if (ShapeGrid)
    {
        if (ShapeGrid->Points.Contains(CellCoord))
        {
            ShapeGrid->Points.Remove(CellCoord);
        }
        else
        {
            ShapeGrid->Points.Add(CellCoord);
        }
    }
}

bool FItemShapeCustomization::IsCellSelected(FIntPoint CellCoord) const
{
    void* StructPtr;
    StructHandle->GetValueData(StructPtr);
    const TArray<FIntPoint>* ShapeGrid = static_cast<TArray<FIntPoint>*>(StructPtr);
    return ShapeGrid ? ShapeGrid->Contains(CellCoord) : false;
}

#undef LOCTEXT_NAMESPACE