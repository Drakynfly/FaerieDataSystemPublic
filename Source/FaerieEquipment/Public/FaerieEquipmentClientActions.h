// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "FaerieItemStack.h"
#include "InventoryDataEnums.h"
#include "Actions/FaerieInventoryClient.h"
#include "FaerieEquipmentClientActions.generated.h"

class UFaerieEquipmentSlot;

// Request that the server set the item in a slot.
USTRUCT(BlueprintType)
struct FFaerieClientAction_RequestSetItemInSlot : public FFaerieClientActionBase
{
	GENERATED_BODY()

	virtual bool Server_Execute(const UFaerieInventoryClient* Client) const override;

	UPROPERTY(BlueprintReadWrite, Category = "RequestSetItemInSlot")
	TObjectPtr<UFaerieEquipmentSlot> Slot = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "RequestSetItemInSlot")
	FFaerieItemStack Stack;
};

// Request that the server move the item in one slot to another, or swaps the items between two slots. If CanSwapSlots
// is false, and FromSlot is empty, nothing will happen.
USTRUCT(BlueprintType)
struct FFaerieClientAction_RequestMoveItemBetweenSlots : public FFaerieClientActionBase
{
	GENERATED_BODY()

	virtual bool Server_Execute(const UFaerieInventoryClient* Client) const override;

	UPROPERTY(BlueprintReadWrite, Category = "RequestMoveItemBetweenSlots")
	TObjectPtr<UFaerieEquipmentSlot> FromSlot = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "RequestMoveItemBetweenSlots")
	TObjectPtr<UFaerieEquipmentSlot> ToSlot = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "RequestMoveItemBetweenSlots")
	bool CanSwapSlots = true;
};

USTRUCT(BlueprintType)
struct FFaerieClientAction_RequestMoveEntryToEquipmentSlot : public FFaerieClientActionBase
{
	GENERATED_BODY()

	virtual bool Server_Execute(const UFaerieInventoryClient* Client) const override;

	UPROPERTY(BlueprintReadWrite, Category = "RequestMoveEntryToEquipmentSlot")
	FInventoryKeyHandle Handle;

	UPROPERTY(BlueprintReadWrite, Category = "RequestMoveEntryToEquipmentSlot")
	TObjectPtr<UFaerieEquipmentSlot> Slot = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "RequestMoveEntryToEquipmentSlot")
	int32 Amount = 1;
};

USTRUCT(BlueprintType)
struct FFaerieClientAction_RequestMoveEquipmentSlotToInventory : public FFaerieClientActionBase
{
	GENERATED_BODY()

	virtual bool Server_Execute(const UFaerieInventoryClient* Client) const override;

	UPROPERTY(BlueprintReadWrite, Category = "RequestMoveEquipmentSlotToInventory")
	TObjectPtr<UFaerieEquipmentSlot> Slot = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "RequestMoveEquipmentSlotToInventory")
	TObjectPtr<UFaerieItemStorage> ToStorage = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "RequestMoveEquipmentSlotToInventory")
	int32 Amount = -1;

	UPROPERTY(BlueprintReadWrite, Category = "RequestMoveEquipmentSlotToInventory")
	EFaerieStorageAddStackBehavior AddStackBehavior = EFaerieStorageAddStackBehavior::AddToAnyStack;
};

USTRUCT(BlueprintType)
struct FFaerieClientAction_RequestMoveEquipmentSlotToSpatialInventory : public FFaerieClientActionBase
{
	GENERATED_BODY()

	virtual bool Server_Execute(const UFaerieInventoryClient* Client) const override;

	UPROPERTY(BlueprintReadWrite, Category = "RequestMoveEquipmentSlotToSpatialInventory")
	FIntPoint TargetPoint = FIntPoint::NoneValue;

	UPROPERTY(BlueprintReadWrite, Category = "RequestMoveEquipmentSlotToSpatialInventory")
	TObjectPtr<UFaerieEquipmentSlot> Slot = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "RequestMoveEquipmentSlotToSpatialInventory")
	TObjectPtr<UFaerieItemStorage> ToStorage = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "RequestMoveEquipmentSlotToSpatialInventory")
	int32 Amount = -1;

	UPROPERTY(BlueprintReadWrite, Category = "RequestMoveEquipmentSlotToSpatialInventory")
	EFaerieStorageAddStackBehavior AddStackBehavior = EFaerieStorageAddStackBehavior::OnlyNewStacks;
};