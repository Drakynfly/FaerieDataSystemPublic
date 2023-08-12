// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "InventoryDataStructs.h"
#include "ItemContainerEvent.h"
#include "FaerieItemStack.h"
#include "NetSupportedObject.h"

#include "InventoryExtensionBase.generated.h"

UENUM()
enum class EEventExtensionResponse : uint8
{
	// The extension does not care/have authority to allow or deny the event.
	NoExplicitResponse,

	// The extension allows the event
	Allowed,

	// The extension forbids the event
	Disallowed
};

/**
 * A modular item container extension. Each extension instance may be registered to multiple UFaerieItemContainerBase
 * objects, so all cached data needs to be sensitive to which container it refers to.
 */
UCLASS(Abstract, HideDropdown, Blueprintable, BlueprintType, EditInlineNew, ClassGroup = "Faerie Inventory", CollapseCategories)
class FAERIEINVENTORY_API UInventoryExtensionBase : public UNetSupportedObject
{
	GENERATED_BODY()

	friend class UInventoryExtensionGroup;
	friend class UFaerieItemContainerBase;

protected:
	/* Called at begin play or when the extension is created during runtime */
	virtual void InitializeExtension(const UFaerieItemContainerBase* Container) {}
	virtual void DeinitializeExtension(const UFaerieItemContainerBase* Container) {}

	/* Does this extension allow this item to be added to the container */
	virtual EEventExtensionResponse AllowsAddition(const UFaerieItemContainerBase* Container, FFaerieItemStackView Stack) { return EEventExtensionResponse::NoExplicitResponse; }

	/* Allows us to react before an item is added */
	virtual void PreAddition(const UFaerieItemContainerBase* Container, FFaerieItemStackView Stack) {}
	/* Allows us to use the key from the last addition */
	virtual void PostAddition(const UFaerieItemContainerBase* Container, const Faerie::FItemContainerEvent& Event) {}

	/* Does this extension allow removal from/of an entry in the container */
	virtual EEventExtensionResponse AllowsRemoval(const UFaerieItemContainerBase* Container, const FEntryKey Key, const FFaerieInventoryTag Reason) const { return EEventExtensionResponse::NoExplicitResponse; }

	/* Allows us to react before an item is removed */
	virtual void PreRemoval(const UFaerieItemContainerBase* Container, const FEntryKey Key, const int32 Removal) {}
	/* Allows us to use the key from the last removal */
	virtual void PostRemoval(const UFaerieItemContainerBase* Container, const Faerie::FItemContainerEvent& Event) {}

	virtual void PostEntryChanged(const UFaerieItemContainerBase* Container, const FEntryKey Key) {}

	/* Request to be the only extension of this class registered to an ItemStorage */
	virtual bool IsUnique() const { return false; }
};

/*
 * A collection of extensions that implements the interface of the base class to defer to others.
 */
UCLASS()
class UInventoryExtensionGroup : public UInventoryExtensionBase
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	//~ UInventoryExtensionBase
	virtual void InitializeExtension(const UFaerieItemContainerBase* Container) override;
	virtual void DeinitializeExtension(const UFaerieItemContainerBase* Container) override;
	virtual EEventExtensionResponse AllowsAddition(const UFaerieItemContainerBase* Container, FFaerieItemStackView Stack) override;
	virtual void PreAddition(const UFaerieItemContainerBase* Container, FFaerieItemStackView Stack) override;
	virtual void PostAddition(const UFaerieItemContainerBase* Container, const Faerie::FItemContainerEvent& Event) override;
	virtual EEventExtensionResponse AllowsRemoval(const UFaerieItemContainerBase* Container, const FEntryKey Key, const FFaerieInventoryTag Reason) const override;
	virtual void PreRemoval(const UFaerieItemContainerBase* Container, const FEntryKey Key, const int32 Removal) override;
	virtual void PostRemoval(const UFaerieItemContainerBase* Container, const Faerie::FItemContainerEvent& Event) override;
	virtual void PostEntryChanged(const UFaerieItemContainerBase* Container, const FEntryKey Key) override;
	//~ UInventoryExtensionBase

private:
	void ForEachExtension(const UFaerieItemContainerBase* Container, TFunction<void(const UFaerieItemContainerBase*, UInventoryExtensionBase*)> Func);

public:
	// Try to add an extension to this storage. This will only fail if the extension pointer is invalid or the extension
	// is Unique, and one already exists of its class.
	bool AddExtension(UInventoryExtensionBase* Extension);

	bool RemoveExtension(UInventoryExtensionBase* Extension);

	// Has extension by class
	UFUNCTION(BlueprintCallable, Category = "Faerie|ExtensionGroup")
	bool HasExtension(TSubclassOf<UInventoryExtensionBase> ExtensionClass) const;

	// Get extension by class
	UFUNCTION(BlueprintCallable, Category = "Faerie|ExtensionGroup", meta = (DeterminesOutputType = ExtensionClass))
	UInventoryExtensionBase* GetExtension(UPARAM(meta = (AllowAbstract = "false")) TSubclassOf<UInventoryExtensionBase> ExtensionClass) const;

	template <typename TExtensionClass> TExtensionClass* GetExtension() const
	{
		return Cast<TExtensionClass>(GetExtension(TExtensionClass::StaticClass()));
	}

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Faerie|ExtensionGroup",
		meta = (DeterminesOutputType = ExtensionClass, DynamicOutputParam = Extension, ExpandBoolAsExecs = "ReturnValue"))
	bool GetExtensionChecked(UPARAM(meta = (AllowAbstract = "false")) TSubclassOf<UInventoryExtensionBase> ExtensionClass,
		UInventoryExtensionBase*& Extension) const;

private:
	// Containers pointing to this group
	UPROPERTY()
	TArray<TObjectPtr<const UFaerieItemContainerBase>> Containers;

	// Subobjects responsible for adding to or customizing container behavior.
	UPROPERTY(EditAnywhere, Replicated, Instanced, NoClear, Category = "ExtensionGroup")
	TArray<TObjectPtr<UInventoryExtensionBase>> Extensions;
};
