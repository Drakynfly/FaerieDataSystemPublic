// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "Blueprint/UserWidget.h"
#include "ActorClasses/FaerieInventoryClient.h"
#include "FaerieItemStorage.h"
#include "InventorySortRule.h"
#include "InventoryContentsBase.generated.h"

class UFaerieItemDataFilter;

DECLARE_LOG_CATEGORY_EXTERN(LogInventoryContents, Log, All)

/**
 *
 */
UCLASS()
class FAERIEINVENTORYCONTENT_API UInventoryContentsBase : public UUserWidget
{
	GENERATED_BODY()

	friend class UInventoryUIAction;

private:
	virtual bool Initialize() override;

	virtual void NativeConstruct() override;

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	void Reset();

	void CreateActions();

	UFUNCTION()
	bool DefaultFilter(const FInventoryEntry& Entry);

	UFUNCTION()
	bool DefaultSort(const FInventoryEntry& A, const FInventoryEntry& B);

protected:
	virtual void NativeEntryAdded(UFaerieItemStorage* Storage, FEntryKey Key);
	virtual void NativeEntryUpdated(UFaerieItemStorage* Storage, FEntryKey Key);
	virtual void NativeEntryRemoved(UFaerieItemStorage* Storage, FEntryKey Key);

public:
	// Set the inventory that will be used when this widget is constructed.
	UFUNCTION(BlueprintCallable, Category = "Inventory Contents|Config")
	void SetLinkedStorage(UFaerieItemStorage* Storage);

	/**
	 * This must be called *after* this widget is added to viewport. Initialization of child widgets cannot be performed
	 * if called while not on-screen.
	 * Calling this multiple times with the same inventory component is intended, and has no performance hit.
	 * It is not necessary to call this at all, unless switching inventories. Calling SetLinkedInventory when created is
	 * sufficient.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory Contents|Config")
	void InitWithInventory(UFaerieItemStorage* Storage);

	UFUNCTION(BlueprintCallable, Category = "Inventory Contents|Config")
	void SetInventoryClient(UFaerieInventoryClient* Client);

	UFUNCTION(BlueprintCallable, Category = "Inventory Contents|Display")
	void AddToSortOrder(FInventoryKey Key, bool WarnIfAlreadyExists);

	UFUNCTION(BlueprintCallable, Category = "Inventory Contents|Display")
	void SetFilterByDelegate(const FBlueprintStorageFilter& Filter, bool bResort = true);

	UFUNCTION(BlueprintCallable, Category = "Inventory Contents|Display")
	void ResetFilter(bool bResort = true);

	UFUNCTION(BlueprintCallable, Category = "Inventory Contents|Display")
	void SetSortRule(UInventorySortRule* Rule, bool bResort = true);

	UFUNCTION(BlueprintCallable, Category = "Inventory Contents|Display")
	void SetSortReverse(bool Reverse, bool bResort = true);

	UFUNCTION(BlueprintCallable, Category = "Inventory Contents|Display")
	void ResetSort(bool bResort = true);

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "Inventory Contents|Display")
	void OnInitWithInventory();

	UFUNCTION(BlueprintImplementableEvent, Category = "Inventory Contents|Display")
	void DisplaySortedEntries();

	UFUNCTION(BlueprintImplementableEvent, Category = "Inventory Contents|Display")
	void OnReset();

	UFUNCTION(BlueprintImplementableEvent, Category = "Inventory Contents|Display")
	void OnKeyAdded(FInventoryKey Key);

	UFUNCTION(BlueprintImplementableEvent, Category = "Inventory Contents|Display")
	void OnKeyUpdated(FInventoryKey Key);

	UFUNCTION(BlueprintImplementableEvent, Category = "Inventory Contents|Display")
	void OnKeyRemoved(FInventoryKey Key);


protected:
	/// ***		SETUP		*** ///

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Instanced, Category = "Display", NoClear)
	TObjectPtr<UFaerieItemDataFilter> DefaultFilterRule;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
	TArray<TSubclassOf<UInventoryUIAction>> ActionClasses;

	// By default all newly added items are sorted into the display order. Disable this when customizing order or filter
	// with OnEntryAdded.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
	bool bAlwaysAddNewToSortOrder = true;


	/// ***		RUNTIME		*** ///

	UPROPERTY(BlueprintReadOnly, Category = "Runtime")
	TArray<FInventoryKey> SortedAndFilteredKeys;

	UPROPERTY(BlueprintReadOnly, Category = "Runtime")
	TObjectPtr<UFaerieItemDataComparator> ActiveSortRule;

	/** The inventory client for this widget to interact with the server.  */
	UPROPERTY(BlueprintReadOnly, Category = "UI Action")
	TWeakObjectPtr<UFaerieInventoryClient> InventoryClient;

	/** The storage this widget is representing.  */
	UPROPERTY(BlueprintReadOnly, Category = "Runtime")
	TWeakObjectPtr<UFaerieItemStorage> ItemStorage;

	UPROPERTY(BlueprintReadOnly, Category = "Runtime")
	TArray<TObjectPtr<UInventoryUIAction>> Actions;

private:
	FFaerieItemStorageNativeQuery Query;

	bool NeedsResort = false;
	bool NeedsReconstructEntries = false;

	UPROPERTY()
	TObjectPtr<UInventoryEntryLiteral> DefaultFilterProxyObject;

	UPROPERTY()
	TObjectPtr<UInventoryEntryLiteral> DefaultSortProxyAObject;

	UPROPERTY()
	TObjectPtr<UInventoryEntryLiteral> DefaultSortProxyBObject;
};