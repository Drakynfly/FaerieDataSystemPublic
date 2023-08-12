// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "FaerieEquipmentSlot.h"

#include "FaerieEquipmentManager.h"
#include "FaerieEquipmentSlotDescription.h"
#include "FaerieInfoObject.h"
#include "FaerieItem.h"
#include "FaerieItemTemplate.h"
#include "InventoryExtensionBase.h"
#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"

FFaerieEquipmentSlotEvents FFaerieEquipmentSlotEvents::FaerieEquipmentSlotEvents;

DEFINE_LOG_CATEGORY(LogFaerieEquipmentSlot)

const UFaerieItem* UFaerieSlotInternalProxy::GetItemObject() const
{
	return Slot->GetItem();
}

int32 UFaerieSlotInternalProxy::GetCopies() const
{
	return 1;
}

TScriptInterface<IFaerieItemOwnerInterface> UFaerieSlotInternalProxy::GetOwner()
{
	return Slot;
}

UFaerieEquipmentSlot::UFaerieEquipmentSlot()
{
	SlotProxy = CreateDefaultSubobject<UFaerieSlotInternalProxy>(TEXT("SlotProxy"));
	SlotProxy->Slot = this;
}

void UFaerieEquipmentSlot::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams SharedParams;
	SharedParams.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, SlotID, SharedParams);
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, SlotDescription, SharedParams);
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, Item, SharedParams);
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, StoredKey, SharedParams);
}

bool UFaerieEquipmentSlot::IsValidKey(const FEntryKey Key) const
{
	return StoredKey == Key;
}

void UFaerieEquipmentSlot::ForEachKey(const TFunctionRef<void(FEntryKey)>& Func) const
{
	if (StoredKey.IsValid())
	{
		Func(StoredKey);
	}
}

FInventoryStack UFaerieEquipmentSlot::GetStack(const FEntryKey Key) const
{
	return 1;
}

FFaerieItemStack UFaerieEquipmentSlot::Release(const FFaerieItemStackView Stack)
{
	if (Item == Stack.Item)
	{
		return { TakeItemFromSlot(), 1 };
	}
	return FFaerieItemStack();
}

bool UFaerieEquipmentSlot::Possess(const FFaerieItemStack Stack)
{
	if (CanSetInSlot(Stack.Item))
	{
		SetItemInSlot(Stack.Item);
		return true;
	}

	return false;
}

UFaerieEquipmentManager* UFaerieEquipmentSlot::GetOuterManager() const
{
	return GetOuterUFaerieEquipmentManager();
}

bool UFaerieEquipmentSlot::CanSetInSlot(const UFaerieItem* TestItem) const
{
	if (!IsValid(TestItem)) return false;

	auto&& Proxy = GetOuterManager()->CachedProxy;
	if (ensure(IsValid(Proxy)))
	{
		GetOuterManager()->CachedProxy->SetValue({TestItem, 1}, nullptr);
		return SlotDescription->Template->TryMatch(Proxy);
	}
	return false;
}

void UFaerieEquipmentSlot::SetItemInSlot(UFaerieItem* InItem)
{
	if (IsValid(Item))
	{
		UE_LOG(LogFaerieEquipmentSlot, Error,
			TEXT("Cannot SetItemInSlot while slot '%s' is filled!"), *SlotDescription->SlotInfo.ObjectName.ToString())
		return;
	}

	Extensions->PreAddition(this, {Item, 1});

	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, Item, this);
	Item = InItem;
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, StoredKey, this);
	StoredKey = NextKey();

	TakeOwnership(Item);

	Faerie::FItemContainerEvent Event;
	Event.Item = Item;
	Event.Amount = 1;
	Event.Success = true;
	Event.Type = FFaerieEquipmentSlotEvents::Get().Set;
	Event.EntryTouched = StoredKey;
	Extensions->PostAddition(this, Event);

	OnItemChanged.Broadcast(this);
}

UFaerieItem* UFaerieEquipmentSlot::TakeItemFromSlot()
{
	if (!IsValid(Item))
	{
		UE_LOG(LogFaerieEquipmentSlot, Error,
			TEXT("Cannot TakeItemFromSlot while slot '%s' is empty!"), *SlotDescription->SlotInfo.ObjectName.ToString())
		return nullptr;
	}

	Extensions->PreRemoval(this, StoredKey, 1);

	ReleaseOwnership(Item);

	Faerie::FItemContainerEvent Event;
	Event.Item = Item;
	Event.Amount = 1;
	Event.Success = true;
	Event.Type = FFaerieEquipmentSlotEvents::Get().Take;
	Event.EntryTouched = StoredKey;

	UFaerieItem* OutItem = Item;
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, Item, this);
	Item = nullptr;
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, StoredKey, this);
	StoredKey = FEntryKey::InvalidKey;

	Extensions->PostRemoval(this, Event);

	OnItemChanged.Broadcast(this);

	return OutItem;
}

FFaerieAssetInfo UFaerieEquipmentSlot::GetSlotInfo() const
{
	return SlotDescription ? SlotDescription->SlotInfo : FFaerieAssetInfo();
}

bool UFaerieEquipmentSlot::IsFilled() const
{
	return IsValid(Item);
}

void UFaerieEquipmentSlot::OnRep_Item()
{
	OnItemChanged.Broadcast(this);
}