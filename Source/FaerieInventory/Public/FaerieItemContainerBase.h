// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "NetSupportedObject.h"
#include "FaerieItemDataProxy.h"
#include "FaerieItemOwnerInterface.h"
#include "InventoryDataStructs.h"
#include "FaerieItemContainerBase.generated.h"

class UItemContainerExtensionBase;

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


	/**------------------------------*/
	/*		 SAVE DATA API			 */
	/**------------------------------*/
public:
	virtual FFaerieContainerSaveData MakeSaveData() const PURE_VIRTUAL(UFaerieItemContainerBase::MakeSaveData, return {}; )
	virtual void LoadSaveData(const FFaerieContainerSaveData& SaveData) PURE_VIRTUAL(UFaerieItemContainerBase::SaveData, )

protected:
	void RavelExtensionData(TMap<FGuid, FInstancedStruct>& Data) const;
	void UnravelExtensionData(const TMap<FGuid, FInstancedStruct>& Data);

	void TryApplyUnclaimedSaveData(UItemContainerExtensionBase* Extension);

	/**------------------------------*/
	/*		 ITEM ENTRY API			 */
	/**------------------------------*/
public:
	virtual bool IsValidKey(FEntryKey Key) const PURE_VIRTUAL(UFaerieItemContainerBase::IsValidKey, return false; )

	// Get a view of an entry
	virtual FFaerieItemStackView View(FEntryKey Key) const PURE_VIRTUAL(UFaerieItemContainerBase::View, return FFaerieItemStackView(); )

	// Creates or retrieves a proxy for an entry
	virtual FFaerieItemProxy Proxy(FEntryKey Key) const PURE_VIRTUAL(UFaerieItemContainerBase::Proxy, return nullptr; )

	// Iterate over and perform a task for each key.
	virtual void ForEachKey(const TFunctionRef<void(FEntryKey)>& Func) const PURE_VIRTUAL(UFaerieItemContainerBase::ForEachKey, ; )

	// Get the stack for a key.
	virtual int32 GetStack(FEntryKey Key) const PURE_VIRTUAL(UFaerieItemContainerBase::GetStack, return 0; )

	// Creates the next unique key for an entry.
	FEntryKey NextKey();

protected:
	virtual void OnItemMutated(const UFaerieItem* Item, const UFaerieItemToken* Token);

	// This function must be called by child classes when binding items to new keys.
	void ReleaseOwnership(UFaerieItem* Item);

	// This function must be called by child classes when releasing a key.
	void TakeOwnership(UFaerieItem* Item);


	/**------------------------------*/
	/*		 EXTENSIONS SYSTEM		 */
	/**------------------------------*/

public:
	// Try to add an extension to this storage. This will only fail if the extension pointer is invalid or the extension
	// is Unique, and one already exists of its class.
	bool AddExtension(UItemContainerExtensionBase* Extension);

	bool RemoveExtension(UItemContainerExtensionBase* Extension);

	// Has extension by class
	UFUNCTION(BlueprintCallable, Category = "Storage|Extensions")
	bool HasExtension(TSubclassOf<UItemContainerExtensionBase> ExtensionClass) const;

	// Get extension by class
	UFUNCTION(BlueprintCallable, Category = "Storage|Extensions", meta = (DeterminesOutputType = ExtensionClass))
	UItemContainerExtensionBase* GetExtension(UPARAM(meta = (AllowAbstract = "false")) TSubclassOf<UItemContainerExtensionBase> ExtensionClass) const;

	template <typename TExtensionClass> TExtensionClass* GetExtension() const
	{
		return Cast<TExtensionClass>(GetExtension(TExtensionClass::StaticClass()));
	}

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Storage|Extensions",
		meta = (DeterminesOutputType = ExtensionClass, DynamicOutputParam = Extension, ExpandBoolAsExecs = "ReturnValue"))
	bool GetExtensionChecked(UPARAM(meta = (AllowAbstract = "false")) TSubclassOf<UItemContainerExtensionBase> ExtensionClass,
		UItemContainerExtensionBase*& Extension) const;

protected:
	// Subobject responsible for adding to or customizing container behavior.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated)
	TObjectPtr<class UItemContainerExtensionGroup> Extensions;

	// Save data for extensions that did not exist on us during unraveling.
	UPROPERTY(Transient)
	TMap<FGuid, FInstancedStruct> UnclaimedExtensionData;

//private:
	// Key tracking starts at 100.
	int32 NextKeyInt = 100;
};