// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "Components/ActorComponent.h"
#include "FaerieItemStorage.h"
#include "InventoryExtensionBase.h"
#include "FaerieInventoryComponent.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogFaerieInventoryComponent, Log, All);

/**
 *	This is the core of the inventory system. The actual component added to actors to allow them to contain item data.
 *	It supports extension objects which customize and add to its functionality, eg: adding capacity limits, or crafting features.
 */
UCLASS(ClassGroup = ("Faerie Inventory"), meta = (BlueprintSpawnableComponent, ChildCannotTick),
	HideCategories = (Cooking, Collision, ComponentTick, Replication, ComponentReplication, Activation, Sockets))
class FAERIEINVENTORY_API UFaerieInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFaerieInventoryComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	//~ UActorComponent
	virtual void BeginPlay() override;
	virtual void ReadyForReplication() override;
	//~ UActorComponent

	void PostEntryAdded(UFaerieItemStorage* Storage, FEntryKey Key);
	void PostEntryChanged(UFaerieItemStorage* Storage, FEntryKey Key);
	void PreEntryRemoved(UFaerieItemStorage* Storage, FEntryKey Key);

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(TArray<FText>& ValidationErrors) override;
#endif

private:
	UFUNCTION()
	void OnRep_ItemStorage();


	/**------------------------------*/
	/*	 INVENTORY API - ALL USERS   */
	/**------------------------------*/

public:
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	UFaerieItemStorage* GetStorage() const { return ItemStorage; }

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Inventory|Extensions",
		meta = (DeterminesOutputType = ExtensionClass, DynamicOutputParam = Extension, ExpandBoolAsExecs = "ReturnValue"))
	bool GetExtensionChecked(UPARAM(meta = (AllowAbstract = "false")) TSubclassOf<UInventoryExtensionBase> ExtensionClass,
		UInventoryExtensionBase*& Extension) const;

	// Add a new extension of the given class, and return the result. If an extension of this class already exists, it
	// will be returned instead.
	UFUNCTION(BlueprintCallable, Category = "Inventory", BlueprintAuthorityOnly, meta = (DeterminesOutputType = "ExtensionClass"))
	UInventoryExtensionBase* AddExtension(TSubclassOf<UInventoryExtensionBase> ExtensionClass);


	/**-------------*/
	/*	 VARIABLES	*/
	/**-------------*/

private:
	UPROPERTY(ReplicatedUsing = "OnRep_ItemStorage")
	TObjectPtr<UFaerieItemStorage> ItemStorage;

	// Subobjects responsible for adding or customizing Inventory behavior.
	UPROPERTY(EditAnywhere, Instanced, NoClear, Category = "Extensions")
	TArray<TObjectPtr<UInventoryExtensionBase>> Extensions;
};