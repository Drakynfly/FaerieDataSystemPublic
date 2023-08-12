// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "FaerieItemContainerBase.h"
#include "ItemContainerEvent.h"
#include "FaerieItemStack.h"
#include "InventoryDataStructs.h"

#include "FaerieItemStorage.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogFaerieItemStorage, Log, All);

using FEntryKeyEventNative = TMulticastDelegate<void(UFaerieItemStorage*, FEntryKey)>;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEntryKeyEvent, UFaerieItemStorage*, Storage, FEntryKey, Key);

using FNativeStorageFilter = TDelegate<bool(const FInventoryEntry&)>;
DECLARE_DYNAMIC_DELEGATE_RetVal_OneParam(bool, FBlueprintStorageFilter, const FInventoryEntry&, Entry);

using FNativeStorageComparator = TDelegate<bool(const FInventoryEntry&, const FInventoryEntry&)>;
DECLARE_DYNAMIC_DELEGATE_RetVal_TwoParams(bool, FBlueprintStorageComparator, const FInventoryEntry&, A, const FInventoryEntry&, B);

struct FFaerieItemStorageNativeQuery
{
	FNativeStorageFilter Filter;
	bool InvertFilter;
	FNativeStorageComparator Sort;
	bool InvertSort;
};

USTRUCT(BlueprintType)
struct FFaerieItemStorageBlueprintQuery
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Faerie|StorageQuery")
	FBlueprintStorageFilter Filter;

	UPROPERTY(BlueprintReadWrite, Category = "Faerie|StorageQuery")
	bool InvertFilter = false;

	UPROPERTY(BlueprintReadWrite, Category = "Faerie|StorageQuery")
	FBlueprintStorageComparator Sort;

	UPROPERTY(BlueprintReadWrite, Category = "Faerie|StorageQuery")
	bool Reverse = false;
};

class UFaerieItemDataProxyBase;
class UInventoryEntryProxy;

/**
 *
 */
UCLASS(BlueprintType)
class FAERIEINVENTORY_API UFaerieItemStorage : public UFaerieItemContainerBase
{
	GENERATED_BODY()

	// Allow the struct that contains our item data to call our content change notification functions.
	friend FInventoryContent;

public:
	//~ UObject
	virtual void PostInitProperties() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostLoad() override;
	//~ UObject

	//~ UFaerieItemContainerBase
	virtual bool IsValidKey(FEntryKey Key) const override;
	virtual void ForEachKey(const TFunctionRef<void(FEntryKey)>& Func) const override;
	virtual FInventoryStack GetStack(FEntryKey Key) const override;
	//~ UFaerieItemContainerBase

	//~ IFaerieItemOwnerInterface
	virtual FFaerieItemStack Release(FFaerieItemStackView Stack) override;
	virtual bool Possess(FFaerieItemStack Stack) override;
	//~ IFaerieItemOwnerInterface


	/**------------------------------*/
	/*	  INTERNAL IMPLEMENTATIONS	 */
	/**------------------------------*/
private:
	const FInventoryEntry& GetEntryImpl(FEntryKey Key) const;

	/**
	* @return Whether or not an entry was found.
	*/
    bool GetEntryImpl(FEntryKey Key, FInventoryEntry& Entry) const;

	// Adds an entry with optional flags for initial state.
	Faerie::FItemContainerEvent AddEntryImpl(const FInventoryEntry& InEntry);
	Faerie::FItemContainerEvent AddEntryFromStackImpl(FFaerieItemStack InStack);
	Faerie::FItemContainerEvent AddEntryFromProxyImpl(UFaerieItemDataProxyBase* InProxy);

	// Internal implementations for removing items, specifying an amount.
	Faerie::FItemContainerEvent RemoveFromEntryImpl(FEntryKey Key, FInventoryStack Amount, FFaerieInventoryTag Reason);
	Faerie::FItemContainerEvent RemoveFromStackImpl(FInventoryKey Key, FInventoryStack Amount, FFaerieInventoryTag Reason);

	void PostContentAdded(const FKeyedInventoryEntry& Entry);
	void PostContentChanged(const FKeyedInventoryEntry& Entry);
	void PreContentRemoved(const FKeyedInventoryEntry& Entry);


	/**------------------------------*/
	/*	  STORAGE API - ALL USERS    */
	/**------------------------------*/
public:
	FEntryKeyEventNative& GetOnKeyAdded() { return OnKeyAddedCallback; }
	FEntryKeyEventNative& GetOnKeyUpdated() { return OnKeyUpdatedCallback; }
	FEntryKeyEventNative& GetOnKeyRemoved() { return OnKeyRemovedCallback; }

	// Convert a entry key into an array of Inventory Keys.
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Storage|Key")
	TArray<FInventoryKey> GetInvKeysForEntry(FEntryKey Key) const;

	// Retrieves all top level and hidden keys. Defaults to skipping over locked keys.
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Storage|Key")
	void GetAllKeys(TArray<FEntryKey>& Keys) const;

	// Retrieve number of entries in storage.
	UFUNCTION(BlueprintCallable, Category = "Storage|Key")
    int32 GetStackCount() const;

	UFUNCTION(BlueprintCallable, Category = "Storage|Key")
	bool ContainsKey(FEntryKey Key) const;

	UFUNCTION(BlueprintCallable, Category = "Storage|Key")
	bool IsValidKey(FInventoryKey Key) const;

	UFUNCTION(BlueprintCallable, Category = "Storage|Key")
	bool ContainsItem(const UFaerieItem* Item) const;

	UFUNCTION(BlueprintCallable, Category = "Storage|Key")
	FEntryKey FindItem(const UFaerieItem* Item) const;

	// Utility function mainly used with inventories that are expected to only contain a single entry, e.g, pickups.
	UFUNCTION(BlueprintCallable, Category = "Storage|Key")
	FInventoryKey GetFirstKey() const;

	/**
	 * Full version
	 * @return Whether or not an entry was found.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Storage|Entry")
	bool GetEntry(FEntryKey Key, FInventoryEntry& Entry) const;

	/**
	 * Get the UInventoryEntryProxy representing an entry of this storage.
	 * The proxy will be cached for quick repeat access.
	 */
	UFUNCTION(BlueprintCallable, Category = "Storage|Cache", meta = (ExpandBoolAsExecs = "ReturnValue"))
	bool GetProxyForEntry(FInventoryKey Key, UInventoryEntryProxy*& Entry);

	/** Get entries en masse */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Storage|Entry")
	void GetEntryArray(const TArray<FEntryKey>& Keys, TArray<FInventoryEntry>& Entries) const;

	// Query function to filter for the first matching entry.
	FKeyedInventoryEntry QueryFirst(const FNativeStorageFilter& Filter) const;

	// Query function to filter and sort for a subsection of contained entries.
	void QueryAll(const FFaerieItemStorageNativeQuery& Query, TArray<FKeyedInventoryEntry>& OutKeys) const;

	// Query function to filter for the first matching entry.
	UFUNCTION(BlueprintCallable, Category = "Storage|Query")
	FEntryKey QueryFirst(const FBlueprintStorageFilter& Filter) const;

	// Query function to filter and sort for a subsection of contained entries.
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Storage|Query")
	void QueryAll(const FFaerieItemStorageBlueprintQuery& Query, TArray<FEntryKey>& OutKeys) const;

	UFUNCTION(BlueprintCallable, Category = "Storage")
	bool CanAddStack(FFaerieItemStackView Stack) const;

	UFUNCTION(BlueprintCallable, Category = "Storage")
	bool CanAddProxy(UFaerieItemDataProxyBase* Proxy) const;

	UFUNCTION(BlueprintCallable, Category = "Storage")
	bool CanRemoveEntry(FEntryKey Key, FFaerieInventoryTag Reason) const;

	UFUNCTION(BlueprintCallable, Category = "Storage")
	bool CanRemoveStack(FInventoryKey Key, FFaerieInventoryTag Reason) const;


	/**------------------------------*/
	/*	 STORAGE API - SERVER ONLY   */
	/**------------------------------*/

	// Add a single raw item.
	UFUNCTION(BlueprintCallable, Category = "Storage", BlueprintAuthorityOnly)
	bool AddEntryFromItemObject(UFaerieItem* ItemObject);

	// Add a single raw item.
	UFUNCTION(BlueprintCallable, Category = "Storage", BlueprintAuthorityOnly)
	bool AddItemStack(FFaerieItemStack ItemStack);

	// Add data from a proxy object.
	UFUNCTION(BlueprintCallable, Category = "Storage", BlueprintAuthorityOnly)
	bool AddEntryFromProxy(UFaerieItemDataProxyBase* Proxy);

	// Add an array of items from proxy objects.
	UFUNCTION(BlueprintCallable, Category = "Storage", BlueprintAuthorityOnly)
	void AddEntriesFromProxy(const TArray<UFaerieItemDataProxyBase*>& Entries);

	/**
	 * Removes the entry with this key, if it exists.
	 * An amount of -1 will remove the entire stack.
	 */
	UFUNCTION(BlueprintCallable, Category = "Storage", BlueprintAuthorityOnly)
	bool RemoveEntry(FEntryKey Key,
		UPARAM(meta = (Categories = "Fae.Inventory.Removal")) FFaerieInventoryTag RemovalTag, const int32 Amount = -1);

	/**
	 * Removes the entry with this key, if it exists.
	 * An amount of -1 will remove the entire stack.
	 */
	UFUNCTION(BlueprintCallable, Category = "Storage", BlueprintAuthorityOnly)
	bool RemoveStack(FInventoryKey Key,
		UPARAM(meta = (Categories = "Fae.Inventory.Removal")) FFaerieInventoryTag RemovalTag, const int32 Amount = -1);

	/**
	 * Removes and returns the entry with this key, if it exists.
	 * An amount of -1 will remove the entire stack.
	 */
	UFUNCTION(BlueprintCallable, Category = "Storage", BlueprintAuthorityOnly)
	bool TakeEntry(FEntryKey Key, FFaerieItemStack& OutStack,
		UPARAM(meta = (Categories = "Fae.Inventory.Removal")) FFaerieInventoryTag RemovalTag, const int32 Amount = -1);

	/**
	 * Removes and returns the entry with this key, if it exists.
	 * An amount of -1 will remove the entire stack.
	 */
	UFUNCTION(BlueprintCallable, Category = "Storage", BlueprintAuthorityOnly)
	bool TakeStack(FInventoryKey Key, FFaerieItemStack& OutStack,
		UPARAM(meta = (Categories = "Fae.Inventory.Removal")) FFaerieInventoryTag RemovalTag, const int32 Amount = -1);

	/**
	 * Clear out the entire contents of the storage.
	 */
	UFUNCTION(BlueprintCallable, Category = "Storage", BlueprintAuthorityOnly)
    void Clear();

	/**
	 * Add an entry from this storage to another, then remove it from this one, optionally move only part of a stack.
	 * @return The key used by the ToStorage to store the entry.
	 */
	UFUNCTION(BlueprintCallable, Category = "Storage", BlueprintAuthorityOnly)
	FEntryKey MoveStack(UFaerieItemStorage* ToStorage, FInventoryKey Key, const int32 Amount = -1);

	UFUNCTION(BlueprintCallable, Category = "Storage", BlueprintAuthorityOnly)
	FEntryKey MoveEntry(UFaerieItemStorage* ToStorage, FEntryKey Key);

	/** Call MoveEntry on all entries in this storage. */
	UFUNCTION(BlueprintCallable, Category = "Storage", BlueprintAuthorityOnly)
	void Dump(UFaerieItemStorage* ToStorage);


	/**-------------*/
	/*	 DELEGATES	*/
	/**-------------*/
protected:
	FEntryKeyEventNative OnKeyAddedCallback;
	FEntryKeyEventNative OnKeyUpdatedCallback;
	FEntryKeyEventNative OnKeyRemovedCallback;

	// Broadcast whenever an entry is added, or a stack amount is increased.
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Transient, Category = "Events")
	FEntryKeyEvent OnKeyAdded;

	// Broadcast whenever data for a key is changed.
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Transient, Category = "Events")
	FEntryKeyEvent OnKeyUpdated;

	// Broadcast whenever an entry is removed entirely, or a stack amount is decreased.
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Transient, Category = "Events")
	FEntryKeyEvent OnKeyRemoved;


	/**-------------*/
	/*	 VARIABLES	*/
	/**-------------*/
private:
	// The internal map containing the contents of the storage.
	UPROPERTY(Replicated, SaveGame)
	FInventoryContent EntryMap;

	// Locally stored Entry Cache objects.
	// This property is transient, mainly so that editor code that calls GetProxyForEntry doesn't need to worry about Caches
	// being left around.
	UPROPERTY(Transient)
	TMap<FInventoryKey, TWeakObjectPtr<UInventoryEntryProxy>> LocalCachedEntries;
};