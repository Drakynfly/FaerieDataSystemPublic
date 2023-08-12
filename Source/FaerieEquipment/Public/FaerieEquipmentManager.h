// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "Components/ActorComponent.h"
#include "FaerieSlotTag.h"

#include "FaerieEquipmentManager.generated.h"

class UFaerieEquipmentSlot;
class UFaerieEquipmentSlotDescription;
class UInventoryExtensionBase;
class UFaerieItemDataStackViewLiteral;


UCLASS(ClassGroup=(Faerie), meta=(BlueprintSpawnableComponent),
	HideCategories = (Cooking, Collision, ComponentTick, Replication, ComponentReplication, Activation, Sockets))
class FAERIEEQUIPMENT_API UFaerieEquipmentManager : public UActorComponent
{
	GENERATED_BODY()

	friend UFaerieEquipmentSlot;

public:
	UFaerieEquipmentManager();

	//~ UObject
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	//~ UObject

	//~ UActorComponent
	virtual void ReadyForReplication() override;
	//~ UActorComponent

public:
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Faerie|EquipmentManager")
	UFaerieEquipmentSlot* AddSlot(FFaerieSlotTag SlotID, UFaerieEquipmentSlotDescription* Description);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Faerie|EquipmentManager")
	bool RemoveSlot(UFaerieEquipmentSlot* Slot);

	UFUNCTION(BlueprintCallable, Category = "Faerie|EquipmentManager")
	TArray<UFaerieEquipmentSlot*> GetSlots() const { return Slots; }

	UFUNCTION(BlueprintCallable, Category = "Faerie|EquipmentManager")
	UFaerieEquipmentSlot* FindSlot(FFaerieSlotTag SlotID) const;

protected:
	// Predefined extensions added to all slots in this manager.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Instanced, NoClear, Category = "Extensions")
	TArray<TObjectPtr<UInventoryExtensionBase>> Extensions;

private:
	UPROPERTY(Replicated, Transient)
	TArray<TObjectPtr<UFaerieEquipmentSlot>> Slots;

	UPROPERTY(Transient)
	TObjectPtr<UFaerieItemDataStackViewLiteral> CachedProxy;
};