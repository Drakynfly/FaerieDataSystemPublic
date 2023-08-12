// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "FaerieItemDataProxy.h"
#include "InventoryDataStructs.h"
#include "LocalInventoryEntryCache.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCacheEvent, UInventoryEntryProxy*, Proxy);

UCLASS(Abstract)
class FAERIEINVENTORY_API UInventoryEntryProxyBase : public UFaerieItemDataProxyBase
{
	GENERATED_BODY()

public:
	/** Get all stacks of this item. */
	UFUNCTION(BlueprintCallable, Category = "Entry Cache")
	TArray<FKeyedStack> GetAllStacks() const;

	/** Get all stacks of this item. */
	UFUNCTION(BlueprintCallable, Category = "Entry Cache")
	FInventoryStack GetStackLimit() const;

	UFUNCTION(BlueprintCallable, Category = "Entry Cache")
	FDateTime GetDateModified() const;

protected:
	virtual FInventoryEntry GetInventoryEntry() const PURE_VIRTUAL(UInventoryEntryProxyBase::GetInventoryEntry, return FInventoryEntry(); )
};

UCLASS(meta = (DontUseGenericSpawnObject = "true"))
class FAERIEINVENTORY_API UInventoryEntryLiteral : public UInventoryEntryProxyBase, public IFaerieItemOwnerInterface
{
	GENERATED_BODY()

public:
	//~ UFaerieItemDataProxyBase
	virtual const UFaerieItem* GetItemObject() const override;
	virtual int32 GetCopies() const override;
	virtual TScriptInterface<IFaerieItemOwnerInterface> GetOwner() override { return this; }
	//~ UFaerieItemDataProxyBase

	//~ IFaerieItemOwnerInterface
	virtual FFaerieItemStack Release(FFaerieItemStackView Stack) override;
	virtual bool Possess(FFaerieItemStack Stack) override;
	//~ IFaerieItemOwnerInterface

protected:
	//~ UInventoryEntryProxyBase
	virtual FInventoryEntry GetInventoryEntry() const override;
	//~ UInventoryEntryProxyBase

public:
	void SetValue(const FInventoryEntry& InEntry);

	UFUNCTION(BlueprintCallable, Category = "Entry Literal")
	static UInventoryEntryLiteral* CreateInventoryEntryLiteral(const FInventoryEntry& Entry);

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Entry Literal")
	FInventoryEntry Entry;
};

/**
 * A implementation of UFaerieItemDataProxyBase that reads from an inventory entry inside an inventory.
 */
UCLASS(meta = (DontUseGenericSpawnObject = "true"))
class FAERIEINVENTORY_API UInventoryEntryProxy : public UInventoryEntryProxyBase
{
	GENERATED_BODY()

	friend class UFaerieItemStorage;

public:
	//~ UFaerieItemDataProxyBase
	virtual const UFaerieItem* GetItemObject() const override;
	virtual int32 GetCopies() const override;
	virtual TScriptInterface<IFaerieItemOwnerInterface> GetOwner() override;
	//~ UFaerieItemDataProxyBase

protected:
	//~ UInventoryEntryProxyBase
	virtual FInventoryEntry GetInventoryEntry() const override;
	//~ UInventoryEntryProxyBase

public:
	const FInventoryKeyHandle& GetHandle() const { return Handle; }

protected:
	void NotifyCreation(const FInventoryKeyHandle& InHandle);
	void NotifyUpdate();
	void NotifyRemoval();

	bool VerifyStatus() const;

public:
	/** Get the stack of this proxy. */
	UFUNCTION(BlueprintCallable, Category = "Entry Cache")
	FInventoryStack GetStack() const;

public:
	UPROPERTY(BlueprintAssignable, Category = "Entry Cache")
	FCacheEvent OnCacheUpdated;

	UPROPERTY(BlueprintAssignable, Category = "Entry Cache")
	FCacheEvent OnCacheRemoved;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Entry Cache")
	FInventoryKeyHandle Handle;

	// Tracks item version locally, so UI knows when to update.
	// -1 means that this Entry has never received a NotifyCreation and is invalid;
	// 0 means that this Entry has received a NotifyCreation, but no NotifyUpdate.
	// Numbers greater increment the Updates we have received.
	UPROPERTY(BlueprintReadOnly, Category = "Entry Cache")
	int32 LocalItemVersion = -1;
};