// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "NetSupportedObject.h"
#include "FaerieItemOwnerInterface.h"
#include "InventoryDataStructs.h"
#include "FaerieItemContainerBase.generated.h"

class UInventoryExtensionBase;

/**
 * Base class for objects that store FaerieItems
 */
UCLASS(Abstract)
class FAERIEINVENTORY_API UFaerieItemContainerBase : public UNetSupportedObject, public IFaerieItemOwnerInterface
{
	GENERATED_BODY()

public:
	UFaerieItemContainerBase();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	//~ IFaerieItemOwnerInterface
	virtual FFaerieItemStack Release(FFaerieItemStackView Stack) override;
	virtual bool Possess(FFaerieItemStack Stack) override;
	//~ IFaerieItemOwnerInterface

public:
	/**------------------------------*/
	/*		 ITEM ENTRY API			 */
	/**------------------------------*/

	virtual bool IsValidKey(FEntryKey Key) const PURE_VIRTUAL(UFaerieItemContainerBase::IsValidKey, return false; )

	// Iterate over and perform a task for each key.
	virtual void ForEachKey(const TFunctionRef<void(FEntryKey)>& Func) const PURE_VIRTUAL(UFaerieItemContainerBase::ForEachKey, ; )

	// Get the stack for a key.
	virtual FInventoryStack GetStack(FEntryKey Key) const PURE_VIRTUAL(UFaerieItemContainerBase::GetStack, return FInventoryStack::EmptyStack; )

	// Creates the next unique key for an entry.
	FEntryKey NextKey();

protected:
	void ReleaseOwnership(const UFaerieItem* Item);

	void TakeOwnership(UFaerieItem* Item);


	/**------------------------------*/
	/*		 EXTENSIONS SYSTEM		 */
	/**------------------------------*/

public:
	// Try to add an extension to this storage. This will only fail if the extension pointer is invalid or the extension
	// is Unique, and one already exists of its class.
	bool AddExtension(UInventoryExtensionBase* Extension);

	bool RemoveExtension(UInventoryExtensionBase* Extension);

	// Has extension by class
	UFUNCTION(BlueprintCallable, Category = "Storage|Extensions")
	bool HasExtension(TSubclassOf<UInventoryExtensionBase> ExtensionClass) const;

	// Get extension by class
	UFUNCTION(BlueprintCallable, Category = "Storage|Extensions", meta = (DeterminesOutputType = ExtensionClass))
	UInventoryExtensionBase* GetExtension(UPARAM(meta = (AllowAbstract = "false")) TSubclassOf<UInventoryExtensionBase> ExtensionClass) const;

	template <typename TExtensionClass> TExtensionClass* GetExtension() const
	{
		return Cast<TExtensionClass>(GetExtension(TExtensionClass::StaticClass()));
	}

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Storage|Extensions",
		meta = (DeterminesOutputType = ExtensionClass, DynamicOutputParam = Extension, ExpandBoolAsExecs = "ReturnValue"))
	bool GetExtensionChecked(UPARAM(meta = (AllowAbstract = "false")) TSubclassOf<UInventoryExtensionBase> ExtensionClass,
		UInventoryExtensionBase*& Extension) const;

protected:
	// Subobject responsible for adding to or customizing container behavior.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated)
	TObjectPtr<class UInventoryExtensionGroup> Extensions;

//private:
	// Key tracking starts at 100.
	int32 NextKeyInt = 100;
};