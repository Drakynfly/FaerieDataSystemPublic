// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "StructUtils/InstancedStruct.h"
#include "Components/ActorComponent.h"
#include "InventoryDataEnums.h"
#include "InventoryDataStructs.h"
#include "FaerieInventoryClient.generated.h"

class UFaerieItemContainerBase;
class UFaerieItemStorage;
class UFaerieEquipmentSlot;
class UFaerieEquipmentManager;
class UFaerieInventoryClient;

USTRUCT()
struct FFaerieClientActionBase
#if CPP
	: public FVirtualDestructor
#endif
{
	GENERATED_BODY()

	/*
	 * Runs on the server when called by UFaerieInventoryClient::RequestExecuteAction.
	 * Use this to implement Client-to-Server edits to item storage.
	 */
	virtual bool Server_Execute(const UFaerieInventoryClient* Client) const PURE_VIRTUAL(FFaerieClientActionBase::Server_Execute, return false; )
};

template<>
struct TStructOpsTypeTraits<FFaerieClientActionBase> : public TStructOpsTypeTraitsBase2<FFaerieClientActionBase>
{
	enum
	{
		WithPureVirtual = true,
	};
};

UENUM()
enum class EFaerieClientRequestBatchType : uint8
{
	// This batch is for sending multiple individual requests at once. Each one will be run, even if some fail.
	Individuals,

	// This batch is for sending a sequence of requests. If one fails, no more will run.
	Sequence
};

/**
 * A component to add to client owned actors, that grants access to inventory functionality.
 */
UCLASS(ClassGroup = ("Faerie"), meta = (BlueprintSpawnableComponent))
class FAERIEINVENTORYCONTENT_API UFaerieInventoryClient : public UActorComponent
{
	GENERATED_BODY()

public:
	UFaerieInventoryClient();

	bool CanAccessContainer(const UFaerieItemContainerBase* Container) const;
	bool CanAccessStorage(const UFaerieItemStorage* Storage) const;
	bool CanAccessEquipment(const UFaerieEquipmentManager* Equipment) const;
	bool CanAccessSlot(const UFaerieEquipmentSlot* Slot) const;

	/**
	 * Sends a request to the server to perform an inventory related edit.
	 * Args must be an InstancedStruct deriving from FFaerieClientActionBase.
	 * This can be called in BP by calling MakeInstancedStruct, and passing any struct into it that is named like "FFaerieClientAction_Request...".
	 *
	 * To define custom actions, derive a struct from FFaerieClientActionBase, and override Server_Execute.
	 */
	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Faerie|InventoryClient")
	void RequestExecuteAction(const TInstancedStruct<FFaerieClientActionBase>& Args);

	/**
	 * Sends requests to the server to perform a batch of inventory related edits.
	 * Each Args struct must be an InstancedStruct deriving from FFaerieClientActionBase.
	 * This can be called in BP by calling MakeInstancedStruct, and passing any struct into it that is named like "FFaerieClientAction_Request...".
	 *
	 * To define custom actions, derive a struct from FFaerieClientActionBase, and override Server_Execute.
	 */
	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Faerie|InventoryClient")
	void RequestExecuteAction_Batch(const TArray<TInstancedStruct<FFaerieClientActionBase>>& Args, EFaerieClientRequestBatchType Type);
};

USTRUCT(BlueprintType)
struct FFaerieClientAction_RequestDeleteEntry : public FFaerieClientActionBase
{
	GENERATED_BODY()

	virtual bool Server_Execute(const UFaerieInventoryClient* Client) const override;

	UPROPERTY(BlueprintReadWrite, Category = "RequestDeleteEntry")
	FInventoryKeyHandle Handle;

	UPROPERTY(BlueprintReadWrite, Category = "RequestDeleteEntry")
	int32 Amount = -1;
};

USTRUCT(BlueprintType)
struct FFaerieClientAction_RequestEjectEntry : public FFaerieClientActionBase
{
	GENERATED_BODY()

	virtual bool Server_Execute(const UFaerieInventoryClient* Client) const override;

	UPROPERTY(BlueprintReadWrite, Category = "RequestEjectEntry")
	FInventoryKeyHandle Handle;

	UPROPERTY(BlueprintReadWrite, Category = "RequestEjectEntry")
	int32 Amount = -1;
};

USTRUCT(BlueprintType)
struct FFaerieClientAction_RequestMoveEntry : public FFaerieClientActionBase
{
	GENERATED_BODY()

	virtual bool Server_Execute(const UFaerieInventoryClient* Client) const override;

	UPROPERTY(BlueprintReadWrite, Category = "RequestMoveEntry")
	FInventoryKeyHandle Handle;

	UPROPERTY(BlueprintReadWrite, Category = "RequestMoveEntry")
	int32 Amount = -1;

	UPROPERTY(BlueprintReadWrite, Category = "RequestMoveEntry")
	TObjectPtr<UFaerieItemStorage> ToStorage = nullptr;
};

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
struct FFaerieClientAction_RequestMoveItemBetweenSpatialSlots : public FFaerieClientActionBase
{
	GENERATED_BODY()

	virtual bool Server_Execute(const UFaerieInventoryClient* Client) const override;

	UPROPERTY(BlueprintReadWrite, Category = "RequestRotateSpatialEntry")
	TObjectPtr<UFaerieItemStorage> Storage = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "RequestMoveSpatialEntry")
	FInventoryKey TargetKey;

	UPROPERTY(BlueprintReadWrite, Category = "RequestMoveSpatialEntry")
	FIntPoint DragEnd = FIntPoint::ZeroValue;
};

USTRUCT(BlueprintType)
struct FFaerieClientAction_RequestRotateSpatialEntry : public FFaerieClientActionBase
{
	GENERATED_BODY()

	virtual bool Server_Execute(const UFaerieInventoryClient* Client) const override;

	UPROPERTY(BlueprintReadWrite, Category = "RequestRotateSpatialEntry")
	TObjectPtr<UFaerieItemStorage> Storage = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "RequestRotateSpatialEntry")
	FInventoryKey Key;

	// @todo should also pass a 90 degree / 180 degree parameter...
};

USTRUCT(BlueprintType)
struct FFaerieClientAction_RequestMoveEquipmentSlotToSpatialInventory : public FFaerieClientActionBase
{
	GENERATED_BODY()

	virtual bool Server_Execute(const UFaerieInventoryClient* Client) const override;
	
	UPROPERTY(BlueprintReadWrite, Category = "RequestMoveEquipmentSlotToSpatialInventory")
	FIntPoint TargetPoint;

	UPROPERTY(BlueprintReadWrite, Category = "RequestMoveEquipmentSlotToSpatialInventory")
	TObjectPtr<UFaerieEquipmentSlot> Slot = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "RequestMoveEquipmentSlotToSpatialInventory")
	TObjectPtr<UFaerieItemStorage> ToStorage = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "RequestMoveEquipmentSlotToSpatialInventory")
	int32 Amount = -1;

	UPROPERTY(BlueprintReadWrite, Category = "RequestMoveEquipmentSlotToSpatialInventory")
	EFaerieStorageAddStackBehavior AddStackBehavior = EFaerieStorageAddStackBehavior::OnlyNewStacks;

};