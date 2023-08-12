// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "WeightedDropCustomization.h"

#include "GenerationStructsLibrary.h"
#include "PropertyEditing.h"

#define LOCTEXT_NAMESPACE "WeightedDropCustomization"

TSharedRef<IPropertyTypeCustomization> FWeightedDropCustomization::MakeInstance()
{
    return MakeShareable(new FWeightedDropCustomization);
}

void FWeightedDropCustomization::CustomizeHeader(const TSharedRef<IPropertyHandle> PropertyHandle,
                                                 FDetailWidgetRow& HeaderRow,
                                                 IPropertyTypeCustomizationUtils& CustomizationUtils)
{
    const TSharedRef<IPropertyHandle> WeightHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FWeightedDrop, Weight)).ToSharedRef();
    const TSharedRef<IPropertyHandle> AdjustedHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FWeightedDrop, AdjustedWeight)).ToSharedRef();
    const TSharedRef<IPropertyHandle> DropHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FWeightedDrop, Drop)).ToSharedRef();
    const TSharedRef<IPropertyHandle> PercentageHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FWeightedDrop, PercentageChanceToDrop)).ToSharedRef();

    const TSharedRef<IPropertyHandle> AssetHandle = DropHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FTableDrop, Asset)).ToSharedRef();
    const TSharedRef<IPropertyHandle> ObjectHandle = AssetHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FFaerieItemSourceObject, Object)).ToSharedRef();

    if (!WeightHandle->IsValidHandle() || !DropHandle->IsValidHandle()) return;

    HeaderRow
        .NameContent()
        [
            PropertyHandle->CreatePropertyNameWidget()
        ]
        .ValueContent()
        .HAlign(HAlign_Fill)
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot()
                .HAlign(HAlign_Fill).AutoWidth()
                .MaxWidth(50)
                .Padding(10)
                [
                    SNew(SVerticalBox)
                    + SVerticalBox::Slot()
                    [
                        WeightHandle->CreatePropertyValueWidget()
                    ]
                    + SVerticalBox::Slot()
                    [
                        AdjustedHandle->CreatePropertyValueWidget()
                    ]
                    + SVerticalBox::Slot()
                    [
                        SNew(STextBlock)
                        .Justification(ETextJustify::Right)
                        .Text_Lambda([PercentageHandle]
                        {
                            float Percentage;
                            PercentageHandle->GetValue(Percentage);

                            // Split percentage into whole value and fractional
                            float Whole;
                            float Decimal = FMath::Modf(Percentage, &Whole);
                            const int32 IntWhole = Whole;

                            // "Bit-shift" fraction part into whole number, to cut off all but two places.
                            Decimal *= 100;
                            const int32 RemainderInt = Decimal;

                            const FString WholeStr = FString::FromInt(IntWhole);
                            FString RemainderString = FString::FromInt(RemainderInt);

                            // Enforce two fractional digits.
                            if (RemainderString.Len() == 1) RemainderString += "0";

                            // Combine and return as percentage text.
                            return FText::FromString(WholeStr + "." + RemainderString + "%");
                        })
                    ]
                ]
            + SHorizontalBox::Slot()
                .HAlign(HAlign_Fill)
                .MaxWidth(400)
                [
                    ObjectHandle->CreatePropertyValueWidget()
                ]
        ];
}

void FWeightedDropCustomization::CustomizeChildren(const TSharedRef<IPropertyHandle> StructPropertyHandle,
                                                   IDetailChildrenBuilder& StructBuilder,
                                                   IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
    const TSharedRef<IPropertyHandle> DropHandle = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FWeightedDrop, Drop)).ToSharedRef();
    const TSharedRef<IPropertyHandle> AssetHandle = DropHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FTableDrop, Asset)).ToSharedRef();
    const TSharedRef<IPropertyHandle> SlotsHandle = DropHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FTableDrop, StaticResourceSlots)).ToSharedRef();
    const TSharedRef<IPropertyHandle> ObjectHandle = AssetHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FFaerieItemSourceObject, Object)).ToSharedRef();

    StructBuilder.AddProperty(SlotsHandle);
}

#undef LOCTEXT_NAMESPACE