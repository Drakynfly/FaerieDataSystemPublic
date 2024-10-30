// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "InstancedStruct.h"
#include "Components/ActorComponent.h"
#include "InventoryDataStructs.h"
#include "FaerieInventoryClient.h"
#include "FaerieSpatialInventoryClient.generated.h"

class UFaerieItemContainerBase;
class UFaerieItemStorage;
class UFaerieEquipmentSlot;
class UFaerieEquipmentManager;
class UFaerieInventoryClient;

/**
 * A component to add to client owned actors, that grants access to inventory functionality.
 */
UCLASS(ClassGroup = ("Faerie"), meta = (BlueprintSpawnableComponent))
class FAERIEINVENTORYCONTENT_API UFaerieSpatialInventoryClient : public UFaerieInventoryClient
{
	GENERATED_BODY()

public:
	UFaerieSpatialInventoryClient();
};

USTRUCT(BlueprintType)
struct FFaerieClientAction_RequestMoveItemBetweenSpatialSlots : public FFaerieClientActionBase
{
	GENERATED_BODY()

	virtual void Server_Execute(const UFaerieInventoryClient* Client) const override;

	UPROPERTY(BlueprintReadWrite, Category = "RequestEjectEntry")
	FInventoryKeyHandle Handle;

	UPROPERTY(BlueprintReadWrite, Category = "RequestMoveItemBetweenSlots")
	int32 SourceSlotIndex;

	UPROPERTY(BlueprintReadWrite, Category = "RequestMoveItemBetweenSlots")
	int32 TargetSlotIndex;

	UPROPERTY(BlueprintReadWrite, Category = "RequestMoveItemBetweenSlots")
	bool CanSwapSlots;
};

USTRUCT(BlueprintType)
struct FFaerieClientAction_RequestRotateSpatialEntry : public FFaerieClientActionBase
{
	GENERATED_BODY()

	virtual void Server_Execute(const UFaerieInventoryClient* Client) const override;

	UPROPERTY(BlueprintReadWrite, Category = "RequestEjectEntry")
	UFaerieItemStorage* Storage;

	UPROPERTY(BlueprintReadWrite, Category = "RequestEjectEntry")
	FEntryKey Key;
	
	UPROPERTY(BlueprintReadWrite, Category = "RequestMoveEntryToEquipmentSlot")
	FIntPoint Loc;
};