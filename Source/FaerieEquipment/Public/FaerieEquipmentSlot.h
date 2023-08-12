// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "FaerieItemContainerBase.h"
#include "FaerieItemDataProxy.h"
#include "FaerieSlotTag.h"
#include "FaerieEquipmentSlot.generated.h"

struct FFaerieAssetInfo;

struct FAERIEEQUIPMENT_API FFaerieEquipmentSlotEvents : public FGameplayTagNativeAdder
{
	FORCEINLINE static const FFaerieEquipmentSlotEvents& Get() { return FaerieEquipmentSlotEvents; }

	// Tag for events where an item is set into a slot
	FFaerieInventoryTag Set;

	// Tag for events where an item is removed from a slot
	FFaerieInventoryTag Take;

protected:
	virtual void AddTags() override
	{
		Set = FFaerieInventoryTag::AddNativeTag(TEXT("Set"),
				"Inventory item data added event");

		Take = FFaerieInventoryTag::AddNativeTag(TEXT("Take"),
				"Inventory item data removed event");
	}

private:
	// Private static object for the global tags. Use the Get() function to access externally.
	static FFaerieEquipmentSlotEvents FaerieEquipmentSlotEvents;
};

DECLARE_LOG_CATEGORY_EXTERN(LogFaerieEquipmentSlot, Log, All)

class UFaerieEquipmentManager;
class UFaerieEquipmentSlotDescription;
class UFaerieItem;

class UFaerieEquipmentSlot;
DECLARE_MULTICAST_DELEGATE_OneParam(FEquipmentSlotEventNative, UFaerieEquipmentSlot*);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEquipmentSlotEvent, UFaerieEquipmentSlot*, Slot);

UCLASS()
class UFaerieSlotInternalProxy : public UFaerieItemDataProxyBase
{
	GENERATED_BODY()

	friend UFaerieEquipmentSlot;

public:
	//~ UFaerieItemDataProxyBase
	virtual const UFaerieItem* GetItemObject() const override;
	virtual int32 GetCopies() const override;
	virtual TScriptInterface<IFaerieItemOwnerInterface> GetOwner() override;
	//~ UFaerieItemDataProxyBase

	TObjectPtr<UFaerieEquipmentSlot> GetSlot() const { return Slot; }

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Faerie|SlotProxy")
	TObjectPtr<UFaerieEquipmentSlot> Slot;
};

/**
 *
 */
UCLASS(BlueprintType, Within = FaerieEquipmentManager)
class FAERIEEQUIPMENT_API UFaerieEquipmentSlot : public UFaerieItemContainerBase
{
	GENERATED_BODY()

	friend UFaerieEquipmentManager;

public:
	UFaerieEquipmentSlot();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	//~ UFaerieItemContainerBase
	virtual bool IsValidKey(FEntryKey Key) const override;
	virtual void ForEachKey(const TFunctionRef<void(FEntryKey)>& Func) const override;
	virtual FInventoryStack GetStack(FEntryKey Key) const override;
	//~ UFaerieItemContainerBase

	//~ IFaerieItemOwnerInterface
	virtual FFaerieItemStack Release(FFaerieItemStackView Stack) override;
	virtual bool Possess(FFaerieItemStack Stack) override;
	//~ IFaerieItemOwnerInterface

public:
	FFaerieSlotTag GetSlotID() const { return SlotID; }
	const UFaerieItem* GetItem() const { return Item; }

	FEquipmentSlotEventNative& GetOnItemChangedNative() { return OnItemChangedNative; }

	UFUNCTION(BlueprintCallable, Category = "Faerie|EquipmentSlot")
	UFaerieEquipmentManager* GetOuterManager() const;

	UFUNCTION(BlueprintCallable, Category = "Faerie|EquipmentSlot")
	bool CanSetInSlot(const UFaerieItem* TestItem) const;

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Faerie|EquipmentSlot")
	void SetItemInSlot(UFaerieItem* InItem);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Faerie|EquipmentSlot")
	UFaerieItem* TakeItemFromSlot();

	UFUNCTION(BlueprintCallable, Category = "Faerie|EquipmentSlot")
	FFaerieAssetInfo GetSlotInfo() const;

	UFUNCTION(BlueprintCallable, Category = "Faerie|EquipmentSlot")
	bool IsFilled() const;

protected:
	UFUNCTION()
	void OnRep_Item();

protected:
	FEquipmentSlotEventNative OnItemChangedNative;

	UPROPERTY(BlueprintAssignable, Category = "Faerie|EquipmentSlot|Events")
	FEquipmentSlotEvent OnItemChanged;

protected:
	// Unique ID for this slot.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Replicated, Category = "EquipmentSlot")
	FFaerieSlotTag SlotID;

	// Info about this slot.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Replicated, Category = "EquipmentSlot")
	TObjectPtr<UFaerieEquipmentSlotDescription> SlotDescription;

	// Current item being contained in this slot.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, ReplicatedUsing = "OnRep_Item", Category = "EquipmentSlot")
	TObjectPtr<UFaerieItem> Item;

	// Proxy used to store the item for use in proxy functions.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "EquipmentSlot")
	TObjectPtr<UFaerieSlotInternalProxy> SlotProxy;

private:
	UPROPERTY(Replicated)
	FEntryKey StoredKey;
};