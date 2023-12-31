﻿// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "FaerieEquipmentSlot.h"
#include "Tokens/FaerieItemStorageToken.h"
#include "FaerieChildSlotToken.generated.h"

class UFaerieEquipmentSlotDescription;

/**
 *
 */
UCLASS(DisplayName = "Token - Child Slot")
class FAERIEEQUIPMENT_API UFaerieChildSlotToken : public UFaerieItemContainerToken
{
	GENERATED_BODY()

public:
	UFaerieChildSlotToken();

	virtual void PostLoad() override;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif

	UFUNCTION(BlueprintCallable, Category = "Faerie|ChildSlot")
	UFaerieEquipmentSlot* GetSlotContainer() const;

protected:
	void OnSlotItemChanged(UFaerieEquipmentSlot* FaerieEquipmentSlot);

protected:
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "EquipmentSlot")
	FFaerieSlotTag SlotID;

	// Description used for the child slot
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "EquipmentSlot")
	TObjectPtr<UFaerieEquipmentSlotDescription> SlotDescription;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "EquipmentSlot")
	bool SingleItemSlot = true;

	UPROPERTY(EditInstanceOnly, Instanced, NoClear, Category = "EquipmentSlot")
	TObjectPtr<UItemContainerExtensionGroup> Extensions;
};