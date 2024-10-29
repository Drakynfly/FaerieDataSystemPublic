// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "InventoryDataStructs.h"
#include "Components/ActorComponent.h"

#include "FaerieInventoryComponent.generated.h"

class UItemContainerExtensionGroup;
class UItemContainerExtensionBase;
class UFaerieItemStorage;

DECLARE_LOG_CATEGORY_EXTERN(LogFaerieInventoryComponent, Log, All);

/**
 *	This is the core of the inventory system. The actual component added to actors to allow them to contain item data.
 *	It supports extension objects which customize and add to its functionality, eg: adding capacity limits, or crafting features.
 */
UCLASS(ClassGroup = ("Faerie"), meta = (BlueprintSpawnableComponent, ChildCannotTick),
	HideCategories = (Collision, ComponentTick, Replication, ComponentReplication, Activation, Sockets, Navigation))
class FAERIEINVENTORY_API UFaerieInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFaerieInventoryComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	//~ UActorComponent
	virtual void PostInitProperties() override;
	virtual void ReadyForReplication() override;
	//~ UActorComponent

	virtual void PostEntryAdded(UFaerieItemStorage* Storage, FEntryKey Key);
	virtual void PostEntryChanged(UFaerieItemStorage* Storage, FEntryKey Key);
	virtual void PreEntryRemoved(UFaerieItemStorage* Storage, FEntryKey Key);


	/**------------------------------*/
	/*	 INVENTORY API - ALL USERS   */
	/**------------------------------*/

public:
	UFUNCTION(BlueprintCallable, Category = "Faerie|InventoryComponent")
	UFaerieItemStorage* GetStorage() const { return ItemStorage; }

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Faerie|InventoryComponent",
		meta = (DeterminesOutputType = ExtensionClass, DynamicOutputParam = Extension, ExpandBoolAsExecs = "ReturnValue"))
	bool GetExtensionChecked(UPARAM(meta = (AllowAbstract = "false")) TSubclassOf<UItemContainerExtensionBase> ExtensionClass,
		UItemContainerExtensionBase*& Extension) const;

	// Add a new extension of the given class, and return the result. If an extension of this class already exists, it
	// will be returned instead.
	UFUNCTION(BlueprintCallable, Category = "Faerie|InventoryComponent", BlueprintAuthorityOnly, meta = (DeterminesOutputType = "ExtensionClass"))
	UItemContainerExtensionBase* AddExtension(TSubclassOf<UItemContainerExtensionBase> ExtensionClass);


	/**-------------*/
	/*	 VARIABLES	*/
	/**-------------*/
private:
	UPROPERTY(Replicated)
	TObjectPtr<UFaerieItemStorage> ItemStorage;

	// Subobjects responsible for adding or customizing Inventory behavior.
	UPROPERTY(EditAnywhere, Instanced, NoClear, Category = "ItemStorage")
	TObjectPtr<UItemContainerExtensionGroup> Extensions;
};