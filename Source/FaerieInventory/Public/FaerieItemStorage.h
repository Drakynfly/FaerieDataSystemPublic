// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "FaerieItemContainerBase.h"
#include "ItemContainerEvent.h"
#include "FaerieItemStack.h"
#include "InventoryDataStructs.h"
#include "StructView.h"

#include "FaerieItemStorage.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogFaerieItemStorage, Log, All);

using FEntryKeyEventNative = TMulticastDelegate<void(UFaerieItemStorage*, FEntryKey)>;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEntryKeyEvent, UFaerieItemStorage*, Storage, FEntryKey, Key);

using FStorageFilterFunc = TFunctionRef<bool(const FFaerieItemProxy&)>;
using FNativeStorageFilter = TDelegate<bool(const FFaerieItemProxy&)>;
DECLARE_DYNAMIC_DELEGATE_RetVal_OneParam(bool, FBlueprintStorageFilter, const FFaerieItemProxy&, Proxy);

using FNativeStorageComparator = TDelegate<bool(const FFaerieItemProxy&, const FFaerieItemProxy&)>;
DECLARE_DYNAMIC_DELEGATE_RetVal_TwoParams(bool, FBlueprintStorageComparator, const FFaerieItemProxy&, A, const FFaerieItemProxy&, B);

struct FFaerieItemStorageNativeQuery
{
	FNativeStorageFilter Filter;
	bool InvertFilter = false;
	FNativeStorageComparator Sort;
	bool InvertSort = false;
};

USTRUCT(BlueprintType)
struct FFaerieItemStorageBlueprintQuery
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Faerie|ItemStorageQuery")
	FBlueprintStorageFilter Filter;

	UPROPERTY(BlueprintReadWrite, Category = "Faerie|ItemStorageQuery")
	bool InvertFilter = false;

	UPROPERTY(BlueprintReadWrite, Category = "Faerie|ItemStorageQuery")
	FBlueprintStorageComparator Sort;

	UPROPERTY(BlueprintReadWrite, Category = "Faerie|ItemStorageQuery")
	bool Reverse = false;
};

class UInventoryEntryProxy;
class UInventoryStackProxy;

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
	virtual FFaerieItemStackView View(FEntryKey Key) const override;
	virtual FFaerieItemProxy Proxy(FEntryKey Key) const override;
	virtual void ForEachKey(const TFunctionRef<void(FEntryKey)>& Func) const override;
	virtual int32 GetStack(FEntryKey Key) const override;

	virtual void OnItemMutated(const UFaerieItem* Item, const UFaerieItemToken* Token) override;
	//~ UFaerieItemContainerBase

	//~ IFaerieItemOwnerInterface
	virtual FFaerieItemStack Release(FFaerieItemStackView Stack) override;
	virtual bool Possess(FFaerieItemStack Stack) override;
	//~ IFaerieItemOwnerInterface


	/**------------------------------*/
	/*	  INTERNAL IMPLEMENTATIONS	 */
	/**------------------------------*/
private:
	FConstStructView GetEntryViewImpl(FEntryKey Key) const;

	UInventoryEntryProxy* GetEntryProxyImpl(FEntryKey Key) const;

	// @todo this copies the entry. Kinda wonky, should be used minimally, if at all.
    void GetEntryImpl(FEntryKey Key, FInventoryEntry& Entry) const;

	// Internal implementations for adding items, in various forms.
	Faerie::Inventory::FEventLog AddEntryImpl(const FInventoryEntry& InEntry);
	Faerie::Inventory::FEventLog AddEntryFromStackImpl(const FFaerieItemStack& InStack);

	// Internal implementations for removing items, specifying an amount.
	Faerie::Inventory::FEventLog RemoveFromEntryImpl(FEntryKey Key, int32 Amount, FFaerieInventoryTag Reason);
	Faerie::Inventory::FEventLog RemoveFromStackImpl(FInventoryKey Key, int32 Amount, FFaerieInventoryTag Reason);

	// FastArray API; used to replicate array changes clientside
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

	FConstStructView GetEntryView(FEntryKey Key) const;

	// Convert an entry key into an array of Inventory Keys.
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Storage|Key")
	TArray<FInventoryKey> GetInvKeysForEntry(FEntryKey Key) const;

	// Gets all entry keys contained in this storage.
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Storage|Key")
	void GetAllKeys(TArray<FEntryKey>& Keys) const;

	// Retrieve the number of entries in storage.
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

	// Utility function mainly used with inventories that are expected to only contain a single entry, e.g., pickups.
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
	bool GetProxyForEntry(FInventoryKey Key, UInventoryStackProxy*& Entry);

	/**
	 * Get the UInventoryEntryProxy representing an entry of this storage.
	 * The proxy will be cached for quick repeat access.
	 */
	UFUNCTION(BlueprintCallable, Category = "Storage|Cache", meta = (ExpandBoolAsExecs = "ReturnValue"))
	bool GetStackProxy(FInventoryKey Key, FFaerieItemProxy& Proxy);

	/** Get entries en masse */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Storage|Entry")
	void GetEntryArray(const TArray<FEntryKey>& Keys, TArray<FInventoryEntry>& Entries) const;

	// Query function to filter for the first matching entry.
	FKeyedInventoryEntry QueryFirst(const FStorageFilterFunc& Filter) const;

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
	bool CanEditEntry(FEntryKey EntryKey);

	UFUNCTION(BlueprintCallable, Category = "Storage")
	bool CanRemoveEntry(FEntryKey Key, FFaerieInventoryTag Reason) const;

	UFUNCTION(BlueprintCallable, Category = "Storage")
	bool CanRemoveStack(FInventoryKey Key, FFaerieInventoryTag Reason) const;


	/**---------------------------------*/
	/*	 STORAGE API - AUTHORITY ONLY   */
	/**---------------------------------*/

	// Add a single raw item.
	UFUNCTION(BlueprintCallable, Category = "Storage", BlueprintAuthorityOnly)
	bool AddEntryFromItemObject(UFaerieItem* ItemObject);

	// Add a single raw item.
	UFUNCTION(BlueprintCallable, Category = "Storage", BlueprintAuthorityOnly)
	bool AddItemStack(FFaerieItemStack ItemStack);

	/**
	 * Removes the entry with this key if it exists.
	 * An amount of -1 will remove the entire stack.
	 */
	UFUNCTION(BlueprintCallable, Category = "Storage", BlueprintAuthorityOnly)
	bool RemoveEntry(FEntryKey Key,
		UPARAM(meta = (Categories = "Fae.Inventory.Removal")) FGameplayTag RemovalTag, const int32 Amount = -1);

	/**
	 * Removes the entry with this key if it exists.
	 * An amount of -1 will remove the entire stack.
	 */
	UFUNCTION(BlueprintCallable, Category = "Storage", BlueprintAuthorityOnly)
	bool RemoveStack(FInventoryKey Key,
		UPARAM(meta = (Categories = "Fae.Inventory.Removal")) FGameplayTag RemovalTag, const int32 Amount = -1);

	/**
	 * Removes and returns the entry with this key if it exists.
	 * An amount of -1 will remove the entire stack.
	 */
	UFUNCTION(BlueprintCallable, Category = "Storage", BlueprintAuthorityOnly)
	bool TakeEntry(FEntryKey Key, FFaerieItemStack& OutStack,
		UPARAM(meta = (Categories = "Fae.Inventory.Removal")) FGameplayTag RemovalTag, const int32 Amount = -1);

	/**
	 * Removes and returns the entry with this key if it exists.
	 * An amount of -1 will remove the entire stack.
	 */
	UFUNCTION(BlueprintCallable, Category = "Storage", BlueprintAuthorityOnly)
	bool TakeStack(FInventoryKey Key, FFaerieItemStack& OutStack,
		UPARAM(meta = (Categories = "Fae.Inventory.Removal")) FGameplayTag RemovalTag, const int32 Amount = -1);

	/**
	 * Clear out the entire contents of the storage.
	 */
	UFUNCTION(BlueprintCallable, Category = "Storage", BlueprintAuthorityOnly)
    void Clear(UPARAM(meta = (Categories = "Fae.Inventory.Removal")) FGameplayTag RemovalTag);

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

	// These properties are transient, mainly so that editor code that calls accesses them don't need to worry about Caches
	// being left around. Using weak pointers here is intentional. We don't want this storage to keep these alive. They
	// should be stored in a strong pointer by whatever requested them, and once nothing needs the proxies, they will die.
	// #todo how will these maps get cleaned up, to they don't accrue hundred of stale ptrs?

	// Locally stored proxies per individual stack.
	UPROPERTY(Transient)
	TMap<FInventoryKey, TWeakObjectPtr<UInventoryStackProxy>> LocalCachedEntries;

	// Locally stored proxies per entry.
	UPROPERTY(Transient)
	TMap<FEntryKey, TWeakObjectPtr<UInventoryEntryProxy>> EntryProxies;
};