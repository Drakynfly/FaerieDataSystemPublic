// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "TableDropCustomization.h"

#include "DetailWidgetRow.h"
#include "FaerieItemSlotUtils.h"
#include "GenerationStructsLibrary.h"
#include "IDetailChildrenBuilder.h"
#include "IPropertyUtilities.h"
#include "PropertyCustomizationHelpers.h"
#include "PropertyHandle.h"
#include "Algo/ForEach.h"

TSharedRef<IPropertyTypeCustomization> FTableDropCustomization::MakeInstance()
{
	return MakeShareable(new FTableDropCustomization());
}

void FTableDropCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow,
	IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	auto&& AssetProp = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FTableDrop, Asset));
	auto&& ObjectProp = AssetProp->GetChildHandle(GET_MEMBER_NAME_CHECKED(FFaerieItemSourceObject, Object));

	HeaderRow
		.NameContent()
		[
			PropertyHandle->CreatePropertyNameWidget()
		]
		.ValueContent()
		[
			ObjectProp->CreatePropertyValueWidget()
		];

	const FSimpleDelegate OnValueChanged = FSimpleDelegate::CreateLambda([&CustomizationUtils]()
	{
		CustomizationUtils.GetPropertyUtilities()->ForceRefresh();
	});

	PropertyHandle->SetOnChildPropertyValueChanged(OnValueChanged);
}

void FTableDropCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle,
	IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	auto&& AssetProp = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FTableDrop, Asset));
	auto&& ObjectProp = AssetProp->GetChildHandle(GET_MEMBER_NAME_CHECKED(FFaerieItemSourceObject, Object));

	auto&& SlotsHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FTableDrop, StaticResourceSlots));

    UObject* ObjectValue = nullptr;
    if (auto&& SoftObjectProperty = CastField<FSoftObjectProperty>(ObjectProp->GetProperty()))
    {
        void* DropAddress;
        ObjectProp->GetValueData(DropAddress);
    	if (DropAddress)
    	{
    		ObjectValue = SoftObjectProperty->LoadObjectPropertyValue(DropAddress);
    	}
    }

	void* SlotsAddress;
	SlotsHandle->GetValueData(SlotsAddress);

	if (!IsValid(ObjectValue))
	{
		return;
	}

	// @todo temp
	ChildBuilder.AddProperty(SlotsHandle.ToSharedRef());
	//return;

	bool ShowSlotsProperty = false;

    // Only display the slots property if the asset is a graph that needs them, or a value has already been set.
    if (ObjectValue->Implements<UFaerieItemSlotInterface>())
    {
    	const FConstStructView SlotsView = UFaerieItemSlotLibrary::GetCraftingSlotsFromObject(ObjectValue);
    	const FFaerieItemCraftingSlots* SlotsPtr = SlotsView.GetPtr<FFaerieItemCraftingSlots>();

    	if (SlotsPtr->RequiredSlots.IsEmpty() &&
    		SlotsPtr->OptionalSlots.IsEmpty())
    	{
    		return;
    	}

    	ShowSlotsProperty = true;

    	auto&& SlotsPropertyPtr = static_cast<TArray<FTableDropResourceSlot>*>(SlotsAddress);
    	FScriptArrayHelper ArrayHelper(CastField<FArrayProperty>(SlotsHandle->GetProperty()), SlotsAddress);

    	// Prefill the map with the slots from the asset:
    	auto SlotIter = [&](const TPair<FFaerieItemSlotHandle, TObjectPtr<UFaerieItemTemplate>>& Slot)
    		{
    			if (FFaerieItemSlotHandle ID = Slot.Key;
					!SlotsPropertyPtr->ContainsByPredicate([ID](const FTableDropResourceSlot& Test)
					{
						return Test.SlotID == ID;
					}))
    			{
    				// Insert blank values for slots that don't exist
    				const int32 Index = ArrayHelper.AddValue();
    				if (Index != INDEX_NONE)
    				{
    					FTableDropResourceSlot* NewValue = SlotsHandle->GetProperty()
							->ContainerPtrToValuePtr<FTableDropResourceSlot>(SlotsAddress, Index);
    					NewValue->SlotID = ID;
    					NewValue->Drop = FInstancedStruct::Make<FTableDrop>();
    				}
    			}
    		};

    	Algo::ForEach(SlotsPtr->RequiredSlots, SlotIter);
    	Algo::ForEach(SlotsPtr->OptionalSlots, SlotIter);
    }
	else // Not a crafting asset
	{
		uint32 NumItems;
		SlotsHandle->AsArray()->GetNumElements(NumItems);

		// If there are items in the map, we should still display the property
		if (NumItems > 0)
		{
			ShowSlotsProperty = true;
		}
	}

	if (ShowSlotsProperty)
	{
		const TSharedRef<FDetailArrayBuilder> ArrayBuilder = MakeShareable(
			new FDetailArrayBuilder(SlotsHandle.ToSharedRef(), true, true, true));

		ArrayBuilder->OnGenerateArrayElementWidget(
			FOnGenerateArrayElementWidget::CreateSP(this, &FTableDropCustomization::OnGenerateElement));

		ChildBuilder.AddCustomBuilder(ArrayBuilder);

		//ChildBuilder.AddProperty(SlotsProp.ToSharedRef());
	}
}

void FTableDropCustomization::OnGenerateElement(TSharedRef<IPropertyHandle, ESPMode::ThreadSafe> PropertyHandle, int32 Index,
	IDetailChildrenBuilder& ChildrenBuilder)
{
	void* DataAddress;
	PropertyHandle->GetValueData(DataAddress);
	auto&& ResourceSlotPtr = static_cast<FTableDropResourceSlot*>(DataAddress);

	auto&& Prop = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FTableDropResourceSlot, Drop));

	ChildrenBuilder.AddCustomRow(FText())
		.NameContent()
		[
			SNew(STextBlock)
				.Text(FText::FromString(ResourceSlotPtr->SlotID.ToString()))
		]
		.ValueContent()
		[
			Prop->CreatePropertyValueWidget()
		];

	// @todo temp
	ChildrenBuilder.AddProperty(Prop.ToSharedRef());
}