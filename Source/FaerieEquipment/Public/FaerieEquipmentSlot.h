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

class UFaerieEquipmentSlotDescription;
class UFaerieItem;

class UFaerieEquipmentSlot;
using FEquipmentSlotEventNative = TMulticastDelegate<void(UFaerieEquipmentSlot*)>;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEquipmentSlotEvent, UFaerieEquipmentSlot*, Slot);


/**
 *
 */
UCLASS(BlueprintType)
class FAERIEEQUIPMENT_API UFaerieEquipmentSlot : public UFaerieItemContainerBase, public IFaerieItemDataProxy
{
	GENERATED_BODY()

	// We friend the only classes allowed to set our SlotDescription
	friend class UFaerieEquipmentManager;
	friend class UFaerieChildSlotToken;

public:
	UFaerieEquipmentSlot();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	//~ UFaerieItemContainerBase
	virtual bool IsValidKey(FEntryKey Key) const override;
	virtual void ForEachKey(const TFunctionRef<void(FEntryKey)>& Func) const override;
	virtual void OnItemMutated(const UFaerieItem* InItem, const UFaerieItemToken* Token) override;

private:
	virtual FFaerieItemStackView View(FEntryKey Key) const override;
	virtual FFaerieItemProxy Proxy(FEntryKey Key) const override;
	virtual int32 GetStack(FEntryKey Key) const override;

public:
	FFaerieItemStackView View() const;
	FFaerieItemProxy Proxy() const;
	int32 GetStack() const;
	//~ UFaerieItemContainerBase

	//~ IFaerieItemDataProxy
	virtual const UFaerieItem* GetItemObject() const override;
	virtual int32 GetCopies() const override;
	virtual bool CanMutate() const override;
	virtual TScriptInterface<IFaerieItemOwnerInterface> GetOwner() const override;
	//~ IFaerieItemDataProxy

	//~ IFaerieItemOwnerInterface
	virtual FFaerieItemStack Release(FFaerieItemStackView Stack) override;
	virtual bool Possess(FFaerieItemStack Stack) override;
	//~ IFaerieItemOwnerInterface

protected:
	virtual void BroadcastChange();
	virtual void BroadcastDataChange();

public:
	FFaerieSlotTag GetSlotID() const { return SlotID; }

	FEquipmentSlotEventNative& GetOnItemChangedNative() { return OnItemChangedNative; }

	// This checks if the stack could ever be contained by this slot, ignoring its current state.
	UFUNCTION(BlueprintCallable, Category = "Faerie|EquipmentSlot")
	bool CouldSetInSlot(FFaerieItemStackView View) const;

	// This checks if the stack can be set to this slot. This is always called during SetItemInSlot, so do not feel the
	// need to always call this first, unless to preemptively check for User-facing purposes.
	UFUNCTION(BlueprintCallable, Category = "Faerie|EquipmentSlot")
	bool CanSetInSlot(FFaerieItemStackView View) const;

	// Use to check beforehand if a removal will go through.
	UFUNCTION(BlueprintCallable, Category = "Faerie|EquipmentSlot")
	bool CanTakeFromSlot(int32 Copies) const;

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Faerie|EquipmentSlot")
	void SetItemInSlot(FFaerieItemStack Stack);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Faerie|EquipmentSlot")
	FFaerieItemStack TakeItemFromSlot(int32 Copies);

	UFUNCTION(BlueprintCallable, Category = "Faerie|EquipmentSlot")
	FFaerieAssetInfo GetSlotInfo() const;

	UFUNCTION(BlueprintCallable, Category = "Faerie|EquipmentSlot")
	bool IsFilled() const;

	UFaerieEquipmentSlot* FindSlot(FFaerieSlotTag SlotTag, bool bRecursive) const;

protected:
	UFUNCTION()
	void OnRep_Item();

protected:
	FEquipmentSlotEventNative OnItemChangedNative;
	FEquipmentSlotEventNative OnItemDataChangedNative;

public:
	// Broadcast when the item filling this slot is removed, or a new item is set.
	UPROPERTY(BlueprintAssignable, Category = "Faerie|EquipmentSlot|Events")
	FEquipmentSlotEvent OnItemChanged;

	// Broadcast when the Item filling this slot has its data mutated. Usually by a sub-item being added/removed.
	UPROPERTY(BlueprintAssignable, Category = "Faerie|EquipmentSlot|Events")
	FEquipmentSlotEvent OnItemDataChanged;

protected:
	// Unique ID for this slot.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Replicated, Category = "Config")
	FFaerieSlotTag SlotID;

	// Info about this slot.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Replicated, Category = "Config")
	TObjectPtr<UFaerieEquipmentSlotDescription> SlotDescription;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Replicated, Category = "Config")
	bool SingleItemSlot;

	// Current item stack being contained in this slot.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, ReplicatedUsing = "OnRep_Item", Category = "State")
	FFaerieItemStack ItemStack;

private:
	// Incremented each time a new item is stored in this stack. Not changed when stack Copies is edited.
	UPROPERTY(Replicated)
	FEntryKey StoredKey;
};