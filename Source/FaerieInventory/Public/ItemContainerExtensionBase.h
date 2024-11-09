// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "NetSupportedObject.h"

#include "InventoryDataStructs.h"

#include "ItemContainerExtensionBase.generated.h"

namespace Faerie::Inventory
{
	class FEventLog;
}

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
class FAERIEINVENTORY_API UItemContainerExtensionBase : public UNetSupportedObject
{
	GENERATED_BODY()

	friend class UItemContainerExtensionGroup;
	friend class UFaerieItemContainerBase;

protected:
	//~ UObject
	virtual void PostDuplicate(EDuplicateMode::Type DuplicateMode) override;
	//~ UObject

	virtual FInstancedStruct MakeSaveData(const UFaerieItemContainerBase* Container) { return {}; }
	virtual void LoadSaveData(const UFaerieItemContainerBase* Container, const FInstancedStruct& SaveData) {}

	/* Called at begin play or when the extension is created during runtime */
	virtual void InitializeExtension(const UFaerieItemContainerBase* Container) {}
	virtual void DeinitializeExtension(const UFaerieItemContainerBase* Container) {}

	/* Does this extension allow this item to be added to the container */
	virtual EEventExtensionResponse AllowsAddition(const UFaerieItemContainerBase* Container, FFaerieItemStackView Stack) { return EEventExtensionResponse::NoExplicitResponse; }

	/* Allows us to react before an item is added */
	virtual void PreAddition(const UFaerieItemContainerBase* Container, FFaerieItemStackView Stack) {}
	/* Allows us to use the key from the last addition */
	virtual void PostAddition(const UFaerieItemContainerBase* Container, const Faerie::Inventory::FEventLog& Event) {}

	/* Does this extension allow removal from/of an entry in the container */
	virtual EEventExtensionResponse AllowsRemoval(const UFaerieItemContainerBase* Container, const FEntryKey Key, const FFaerieInventoryTag Reason) const { return EEventExtensionResponse::NoExplicitResponse; }

	/* Allows us to react before an item is removed */
	virtual void PreRemoval(const UFaerieItemContainerBase* Container, const FEntryKey Key, const int32 Removal) {}
	/* Allows us to use the key from the last removal */
	virtual void PostRemoval(const UFaerieItemContainerBase* Container, const Faerie::Inventory::FEventLog& Event) {}

	virtual void PostEntryChanged(const UFaerieItemContainerBase* Container, const FEntryKey Key) {}

public:
	void SetIdentifier(const FGuid* GuidToUse = nullptr);

	FGuid GetIdentifier() const { return Identifier; }

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Extension")
	FGuid Identifier;
};

/*
 * A collection of extensions that implements the interface of the base class to defer to others.
 */
UCLASS()
class FAERIEINVENTORY_API UItemContainerExtensionGroup final : public UItemContainerExtensionBase
{
	GENERATED_BODY()

	friend class UFaerieItemContainerBase;

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void PostLoad() override;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif

	//~ UItemContainerExtensionBase
	virtual void InitializeExtension(const UFaerieItemContainerBase* Container) override;
	virtual void DeinitializeExtension(const UFaerieItemContainerBase* Container) override;
	virtual EEventExtensionResponse AllowsAddition(const UFaerieItemContainerBase* Container, FFaerieItemStackView Stack) override;
	virtual void PreAddition(const UFaerieItemContainerBase* Container, FFaerieItemStackView Stack) override;
	virtual void PostAddition(const UFaerieItemContainerBase* Container, const Faerie::Inventory::FEventLog& Event) override;
	virtual EEventExtensionResponse AllowsRemoval(const UFaerieItemContainerBase* Container, const FEntryKey Key, const FFaerieInventoryTag Reason) const override;
	virtual void PreRemoval(const UFaerieItemContainerBase* Container, const FEntryKey Key, const int32 Removal) override;
	virtual void PostRemoval(const UFaerieItemContainerBase* Container, const Faerie::Inventory::FEventLog& Event) override;
	virtual void PostEntryChanged(const UFaerieItemContainerBase* Container, const FEntryKey Key) override;
	//~ UItemContainerExtensionBase

private:
	void ForEachExtension(const TFunctionRef<void(UItemContainerExtensionBase*)>& Func);

public:
	// Try to add an extension to this storage. This will only fail if the extension pointer is invalid or the extension
	// is Unique, and one already exists of its class.
	bool AddExtension(UItemContainerExtensionBase* Extension);

	bool RemoveExtension(UItemContainerExtensionBase* Extension);

	// Has extension by class
	UFUNCTION(BlueprintCallable, Category = "Faerie|ExtensionGroup")
	bool HasExtension(TSubclassOf<UItemContainerExtensionBase> ExtensionClass) const;

	// Get extension by class
	UFUNCTION(BlueprintCallable, Category = "Faerie|ExtensionGroup", meta = (DeterminesOutputType = ExtensionClass))
	UItemContainerExtensionBase* GetExtension(UPARAM(meta = (AllowAbstract = "false")) TSubclassOf<UItemContainerExtensionBase> ExtensionClass) const;

	template <typename TExtensionClass> TExtensionClass* GetExtension() const
	{
		return Cast<TExtensionClass>(GetExtension(TExtensionClass::StaticClass()));
	}

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Faerie|ExtensionGroup",
		meta = (DeterminesOutputType = ExtensionClass, DynamicOutputParam = Extension, ExpandBoolAsExecs = "ReturnValue"))
	bool GetExtensionChecked(UPARAM(meta = (AllowAbstract = "false")) TSubclassOf<UItemContainerExtensionBase> ExtensionClass,
		UItemContainerExtensionBase*& Extension) const;

private:
	// Containers pointing to this group
	UPROPERTY(Transient)
	TSet<TWeakObjectPtr<const UFaerieItemContainerBase>> Containers;

	// Subobjects responsible for adding to or customizing container behavior.
	UPROPERTY(EditAnywhere, Replicated, Instanced, NoClear, Transient, Category = "ExtensionGroup")
	TArray<TObjectPtr<UItemContainerExtensionBase>> Extensions;
};