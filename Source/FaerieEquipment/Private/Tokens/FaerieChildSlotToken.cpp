// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "Tokens/FaerieChildSlotToken.h"
#include "FaerieEquipmentSlot.h"
#include "ItemContainerExtensionBase.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#include "FaerieEquipmentSlotDescription.h"
#endif

#include UE_INLINE_GENERATED_CPP_BY_NAME(FaerieChildSlotToken)

UFaerieChildSlotToken::UFaerieChildSlotToken()
{
	ItemContainer = CreateDefaultSubobject<UFaerieEquipmentSlot>(FName{TEXTVIEW("ItemContainer")});
	Extensions = CreateDefaultSubobject<UItemContainerExtensionGroup>(FName{TEXTVIEW("Extensions")});
	Extensions->SetIdentifier();
	ItemContainer->AddExtension(Extensions);
}

void UFaerieChildSlotToken::PostLoad()
{
	Super::PostLoad();

	if (!IsTemplate())
	{
		if (auto&& Slot = GetSlotContainer())
		{
			Slot->SlotID = SlotID;
			Slot->SlotDescription = SlotDescription;
			Slot->SingleItemSlot = SingleItemSlot;
			Slot->OnItemChangedNative.AddUObject(this, &ThisClass::OnSlotItemChanged);
		}
	}
}

#if WITH_EDITOR

#define LOCTEXT_NAMESPACE "ValidateFaerieChildSlotToken"

EDataValidationResult UFaerieChildSlotToken::IsDataValid(FDataValidationContext& Context) const
{
	if (!SlotID.IsValid())
	{
		Context.AddError(LOCTEXT("InvalidSlotID", "Must have valid SlotID set!"));
	}

	if (!IsValid(SlotDescription))
	{
		Context.AddError(LOCTEXT("InvalidSlotDescription", "Must have valid SlotDescription set!"));
	}

	if (Context.GetNumErrors())
	{
		return EDataValidationResult::Invalid;
	}
	return Super::IsDataValid(Context);
}

#undef LOCTEXT_NAMESPACE

#endif

UFaerieEquipmentSlot* UFaerieChildSlotToken::GetSlotContainer() const
{
	return Cast<UFaerieEquipmentSlot>(ItemContainer);
}

void UFaerieChildSlotToken::OnSlotItemChanged(UFaerieEquipmentSlot* FaerieEquipmentSlot)
{
	NotifyOuterOfChange();
}