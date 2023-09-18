// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "FaerieItemDataProxy.h"
#include "InventoryDataStructs.h"
#include "LocalInventoryEntryCache.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCacheEvent, UInventoryEntryStorageProxy*, Proxy);

UCLASS(Abstract)
class FAERIEINVENTORY_API UInventoryEntryProxyBase : public UObject, public IFaerieItemDataProxy
{
	GENERATED_BODY()

public:
	/** Get all stacks of this item. */
	UFUNCTION(BlueprintCallable, Category = "Entry Cache")
	TArray<FKeyedStack> GetAllStacks() const;

	/** Get all stacks of this item. */
	UFUNCTION(BlueprintCallable, Category = "Entry Cache")
	int32 GetStackLimit() const;

protected:
	virtual FInventoryEntry GetInventoryEntry() const PURE_VIRTUAL(UInventoryEntryProxyBase::GetInventoryEntry, return FInventoryEntry(); )
};

UCLASS(Abstract)
class UInventoryEntryStorageProxy : public UInventoryEntryProxyBase
{
	GENERATED_BODY()

public:
	//~ IFaerieItemDataProxy
	virtual const UFaerieItem* GetItemObject() const override;
	virtual int32 GetCopies() const override;
	virtual bool CanMutate() const override;
	virtual TScriptInterface<IFaerieItemOwnerInterface> GetOwner() const override;
	//~ IFaerieItemDataProxy

protected:
	//~ UInventoryEntryProxyBase
	virtual FInventoryEntry GetInventoryEntry() const override final;
	//~ UInventoryEntryProxyBase

	void NotifyCreation();
	void NotifyUpdate();
	void NotifyRemoval();

	bool VerifyStatus() const;

	virtual UFaerieItemStorage* GetStorage() const PURE_VIRTUAL(UInventoryEntryStorageProxy::GetStorage, return nullptr; )
	virtual FEntryKey GetKey() const PURE_VIRTUAL(UInventoryEntryStorageProxy::GetKey, return FEntryKey(); )

public:
	UPROPERTY(BlueprintAssignable, Category = "Entry Cache")
	FCacheEvent OnCacheUpdated;

	UPROPERTY(BlueprintAssignable, Category = "Entry Cache")
	FCacheEvent OnCacheRemoved;

protected:
	// Tracks item version locally, so UI knows when to update.
	// -1 means that this Entry has never received a NotifyCreation and is invalid;
	// 0 means that this Entry has received a NotifyCreation, but no NotifyUpdate.
	// Numbers greater increment the Updates we have received.
	UPROPERTY(BlueprintReadOnly, Category = "Entry Cache")
	int32 LocalItemVersion = -1;
};

class UFaerieItemStorage;

UCLASS(meta = (DontUseGenericSpawnObject = "true"))
class UInventoryEntryProxy : public UInventoryEntryStorageProxy
{
	GENERATED_BODY()

	friend UFaerieItemStorage;

protected:
	virtual UFaerieItemStorage* GetStorage() const override;
	virtual FEntryKey GetKey() const override;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TWeakObjectPtr<UFaerieItemStorage> ItemStorage;

	UPROPERTY(BlueprintReadOnly, Category = "Entry Cache")
	FEntryKey Key;
};

/**
 * A implementation of UInventoryEntryProxyBase that reads from an inventory stack inside an inventory.
 */
UCLASS(meta = (DontUseGenericSpawnObject = "true"))
class UInventoryStackProxy : public UInventoryEntryStorageProxy
{
	GENERATED_BODY()

	friend UFaerieItemStorage;

public:
	//~ IFaerieItemDataProxy
	virtual int32 GetCopies() const override;
	//~ IFaerieItemDataProxy

protected:
	//~ UInventoryEntryStorageProxy
	virtual UFaerieItemStorage* GetStorage() const override;
	virtual FEntryKey GetKey() const override;
	//~ UInventoryEntryStorageProxy

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Entry Cache")
	FInventoryKeyHandle Handle;
};