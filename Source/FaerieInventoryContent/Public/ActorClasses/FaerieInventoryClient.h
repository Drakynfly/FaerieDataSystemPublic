// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "Components/ActorComponent.h"
#include "InventoryDataStructs.h"
#include "FaerieInventoryClient.generated.h"

struct FFaerieInventoryUserTag;
class UFaerieItemContainerBase;
class UFaerieItemStorage;
class UFaerieEquipmentSlot;
class UFaerieEquipmentManager;

/**
 * A component to add to client owned actors, that grants access to inventory functionality.
 */
UCLASS(ClassGroup = ("Faerie"), meta = (BlueprintSpawnableComponent))
class FAERIEINVENTORYCONTENT_API UFaerieInventoryClient : public UActorComponent
{
	GENERATED_BODY()

public:
	UFaerieInventoryClient();

protected:
	bool CanAccessContainer(UFaerieItemContainerBase* Container) const;
	bool CanAccessStorage(UFaerieItemStorage* Storage) const;
	bool CanAccessEquipment(UFaerieEquipmentManager* Equipment) const;
	bool CanAccessSlot(UFaerieEquipmentSlot* Slot) const;

public:
	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Faerie|InventoryClient")
	void RequestDeleteEntry(const FInventoryKeyHandle Handle, const int32 Amount = -1);

	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Faerie|InventoryClient")
	void RequestEjectEntry(const FInventoryKeyHandle Handle, const int32 Amount = -1);

	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Faerie|InventoryClient")
	void RequestMoveEntry(const FInventoryKeyHandle Handle, const int32 Amount = -1, UFaerieItemStorage* ToStorage = nullptr);

	UFUNCTION(BlueprintCallable, Server, Unreliable, Category = "Faerie|InventoryClient")
	void RequestMarkStackWithTag(const FInventoryKeyHandle Handle, FFaerieInventoryUserTag Tag);

	UFUNCTION(BlueprintCallable, Server, Unreliable, Category = "Faerie|InventoryClient")
	void RequestClearTagFromStack(const FInventoryKeyHandle Handle, FFaerieInventoryUserTag Tag);

	// Request that the server set the item in a slot.
	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Faerie|EquipmentClient")
	void RequestSetItemInSlot(UFaerieEquipmentSlot* Slot, FFaerieItemStack Stack);

	// Request that the server move the item in one slot to another, or swaps the items between two slots. If CanSwapSlots
	// is false, and FromSlot is empty, nothing will happen.
	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Faerie|EquipmentClient")
	void RequestMoveItemBetweenSlots(UFaerieEquipmentSlot* FromSlot, UFaerieEquipmentSlot* ToSlot, bool CanSwapSlots);

	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Faerie|InventoryClient")
	void RequestMoveEntryToEquipmentSlot(const FInventoryKeyHandle Handle, UFaerieEquipmentSlot* Slot);

	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Faerie|InventoryClient")
	void RequestMoveEquipmentSlotToInventory(UFaerieEquipmentSlot* Slot, UFaerieItemStorage* ToStorage, int32 Amount = -1);
};