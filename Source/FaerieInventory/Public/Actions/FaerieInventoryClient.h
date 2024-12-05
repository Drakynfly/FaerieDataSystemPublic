// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "StructUtils/InstancedStruct.h"
#include "Components/ActorComponent.h"
#include "InventoryDataStructs.h"
#include "FaerieInventoryClient.generated.h"

class UFaerieItemContainerBase;
class UFaerieItemStorage;
class UFaerieInventoryClient;

USTRUCT()
struct FAERIEINVENTORY_API FFaerieClientActionBase
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
class FAERIEINVENTORY_API UFaerieInventoryClient : public UActorComponent
{
	GENERATED_BODY()

public:
	UFaerieInventoryClient();

	bool CanAccessContainer(const UFaerieItemContainerBase* Container) const;
	bool CanAccessStorage(const UFaerieItemStorage* Storage) const;

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

	UPROPERTY(BlueprintReadWrite, Category = "DeleteEntry")
	FInventoryKeyHandle Handle;

	UPROPERTY(BlueprintReadWrite, Category = "DeleteEntry")
	int32 Amount = -1;
};

USTRUCT(BlueprintType)
struct FFaerieClientAction_RequestMoveEntry : public FFaerieClientActionBase
{
	GENERATED_BODY()

	virtual bool Server_Execute(const UFaerieInventoryClient* Client) const override;

	UPROPERTY(BlueprintReadWrite, Category = "MoveEntry")
	FInventoryKeyHandle Handle;

	UPROPERTY(BlueprintReadWrite, Category = "MoveEntry")
	int32 Amount = -1;

	UPROPERTY(BlueprintReadWrite, Category = "MoveEntry")
	TObjectPtr<UFaerieItemStorage> ToStorage = nullptr;
};

USTRUCT(BlueprintType)
struct FFaerieClientAction_MergeStacks : public FFaerieClientActionBase
{
	GENERATED_BODY()

	virtual bool Server_Execute(const UFaerieInventoryClient* Client) const override;

	UPROPERTY(BlueprintReadWrite, Category = "MergeStacks")
	TObjectPtr<UFaerieItemStorage> Storage = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "MergeStacks")
	FEntryKey Entry;

	UPROPERTY(BlueprintReadWrite, Category = "MergeStacks")
	FStackKey FromStack;

	UPROPERTY(BlueprintReadWrite, Category = "MergeStacks")
	FStackKey ToStack;

	// Amount to move from A to B. If equal to -1, the entire stack will attempt to merge.
	UPROPERTY(BlueprintReadWrite, Category = "MergeStacks")
	int32 Amount = -1;
};

USTRUCT(BlueprintType)
struct FFaerieClientAction_RequestSplitStack : public FFaerieClientActionBase
{
	GENERATED_BODY()

	virtual bool Server_Execute(const UFaerieInventoryClient* Client) const override;

	UPROPERTY(BlueprintReadWrite, Category = "SplitStack")
	TObjectPtr<UFaerieItemStorage> Storage = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "SplitStack")
	FInventoryKey Key;

	UPROPERTY(BlueprintReadWrite, Category = "SplitStack")
	int32 Amount = 1;
};