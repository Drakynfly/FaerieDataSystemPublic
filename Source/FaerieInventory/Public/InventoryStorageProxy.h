// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "FaerieItemDataProxy.h"
#include "InventoryDataStructs.h"
#include "InventoryStorageProxy.generated.h"

class UFaerieClientItemProxy;
class UInventoryEntryStorageProxy;
using FEntryStorageProxyEvent = TMulticastDelegate<void(UInventoryEntryStorageProxy*)>;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCacheEvent, UInventoryEntryStorageProxy*, Proxy);

using FClientItemProxyNativeEvent = TMulticastDelegate<void(UFaerieClientItemProxy *)>;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FClientItemProxyEvent, UFaerieClientItemProxy*, Proxy);

UCLASS(Abstract)
class UInventoryEntryProxyBase : public UObject, public IFaerieItemDataProxy
{
	GENERATED_BODY()

public:
	/** Get all stacks for this item. */
	UFUNCTION(BlueprintCallable, Category = "Entry Cache")
	FAERIEINVENTORY_API TArray<FKeyedStack> GetAllStacks() const;

	/** Get the stack limit of this item. */
	UFUNCTION(BlueprintCallable, Category = "Entry Cache")
	FAERIEINVENTORY_API int32 GetStackLimit() const;

protected:
	virtual FInventoryEntryView GetInventoryEntry() const PURE_VIRTUAL(UInventoryEntryProxyBase::GetInventoryEntry, return FInventoryEntryView(); )
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

	//~ UInventoryEntryProxyBase
	virtual FInventoryEntryView GetInventoryEntry() const override final;
	//~ UInventoryEntryProxyBase

	FAERIEINVENTORY_API UFaerieItemStorage* GetStorage() const { return ItemStorage.Get(); }
	FAERIEINVENTORY_API int32 GetItemVersion() const { return LocalItemVersion; }
	FAERIEINVENTORY_API virtual FEntryKey GetKey() const PURE_VIRTUAL(UInventoryEntryStorageProxy::GetKey, return FEntryKey(); )

	FAERIEINVENTORY_API FEntryStorageProxyEvent::RegistrationType& GetOnCacheUpdated() { return OnCacheUpdatedNative; }
	FAERIEINVENTORY_API FEntryStorageProxyEvent::RegistrationType& GetOnCacheRemoved() { return OnCacheRemovedNative; }

protected:
	void NotifyCreation();
	void NotifyUpdate();
	void NotifyRemoval();

	bool VerifyStatus() const;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FCacheEvent OnCacheUpdated;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FCacheEvent OnCacheRemoved;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TWeakObjectPtr<UFaerieItemStorage> ItemStorage;

	// Tracks item version locally, so UI knows when to update.
	// -1 means that this Entry has never received a NotifyCreation and is invalid;
	// 0 means that this Entry has received a NotifyCreation, but no NotifyUpdate.
	// Numbers greater increment the Updates we have received.
	UPROPERTY(BlueprintReadOnly, Category = "Entry Cache")
	int32 LocalItemVersion = -1;

private:
	FEntryStorageProxyEvent OnCacheUpdatedNative;
	FEntryStorageProxyEvent OnCacheRemovedNative;
};

class UFaerieItemStorage;

/**
 * An implementation of UInventoryEntryStorageProxy that reads from an inventory entry inside an item storage.
 */
UCLASS(meta = (DontUseGenericSpawnObject = "true"))
class FAERIEINVENTORY_API UInventoryEntryProxy : public UInventoryEntryStorageProxy
{
	GENERATED_BODY()

	friend UFaerieItemStorage;

public:
	//~ UInventoryEntryStorageProxy
	virtual FEntryKey GetKey() const override;
	//~ UInventoryEntryStorageProxy

protected:
	UPROPERTY(BlueprintReadOnly, Category = "EntryProxy")
	FEntryKey Key;
};

/**
 * An implementation of UInventoryEntryStorageProxy that reads from an inventory stack inside an item storage.
 */
UCLASS(meta = (DontUseGenericSpawnObject = "true"))
class FAERIEINVENTORY_API UInventoryStackProxy : public UInventoryEntryStorageProxy
{
	GENERATED_BODY()

	friend UFaerieItemStorage;

public:
	//~ IFaerieItemDataProxy
	virtual int32 GetCopies() const override;
	//~ IFaerieItemDataProxy

	//~ UInventoryEntryStorageProxy
	virtual FEntryKey GetKey() const override;
	//~ UInventoryEntryStorageProxy

	UFUNCTION(BlueprintCallable, Category = "Faerie|StackProxy")
	FInventoryKeyHandle GetHandle() const;

protected:
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "StackProxy")
	FInventoryKey Key;
};

UCLASS(BlueprintType, Blueprintable)
class UFaerieClientItemProxy : public UInventoryEntryStorageProxy
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category="Faerie|ClientItemProxy")
	FInventoryKey& GetInventoryKey() { return Key; }

	UFUNCTION(BlueprintCallable, Category = "Faerie|ClientItemProxy")
	void InitializeProxy(UFaerieItemStorage* InItemStorage, const FInventoryKey InInventoryKey);
	UPROPERTY(BlueprintAssignable)
	FClientItemProxyEvent OnClientItemUpdated;
protected:
	UPROPERTY(BlueprintReadOnly)
	FInventoryKey Key;
	
	void NotifyCreation(UFaerieItemStorage* Storage, const FEntryKey InKey);
private:
	FClientItemProxyNativeEvent OnClientItemUpdatedNative;

};
