// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "FaerieEquipmentSlot.h"

#include "FaerieEquipmentSlotDescription.h"
#include "FaerieInfoObject.h"
#include "FaerieItem.h"
#include "FaerieItemTemplate.h"
#include "ItemContainerEvent.h"
#include "ItemContainerExtensionBase.h"
#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"
#include "Tokens/FaerieChildSlotToken.h"

FFaerieEquipmentSlotEvents FFaerieEquipmentSlotEvents::FaerieEquipmentSlotEvents;

DEFINE_LOG_CATEGORY(LogFaerieEquipmentSlot)

UFaerieEquipmentSlot::UFaerieEquipmentSlot()
{
	SingleItemSlot = true;
}

void UFaerieEquipmentSlot::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Config members only need to replicate once
	DOREPLIFETIME_CONDITION(ThisClass, SlotID, COND_InitialOnly);
	DOREPLIFETIME_CONDITION(ThisClass, SlotDescription, COND_InitialOnly);
	DOREPLIFETIME_CONDITION(ThisClass, SingleItemSlot, COND_InitialOnly);

	// State members are push based
	FDoRepLifetimeParams SharedParams;
	SharedParams.bIsPushBased = true;
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, ItemStack, SharedParams);
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, StoredKey, SharedParams);
}

//~ UFaerieItemContainerBase
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

void UFaerieEquipmentSlot::OnItemMutated(const UFaerieItem* InItem, const UFaerieItemToken* Token)
{
	Super::OnItemMutated(InItem, Token);
	check(ItemStack.Item == InItem);

	// Since we should be the only owner of Item, it shouldn't change under us, unless it has a child slot that has an item
	// added or removed from it. Verify this by checking the class of the token.
	// If at somepoint this ensure trips, first assume that something has wrongly mutated an item inside a slot (which
	// shouldn't happen) or that a slot is wrongly holding onto something it shouldn't.
	ensure(Token->IsA<UFaerieItemContainerToken>());

	OnItemDataChanged.Broadcast(this);
}

FFaerieItemStackView UFaerieEquipmentSlot::View(FEntryKey Key) const
{
	return ItemStack;
}

FFaerieItemStackView UFaerieEquipmentSlot::View() const
{
	return ItemStack;
}

FFaerieItemProxy UFaerieEquipmentSlot::Proxy(FEntryKey Key) const
{
	return this;
}

FFaerieItemProxy UFaerieEquipmentSlot::Proxy() const
{
	return this;
}

int32 UFaerieEquipmentSlot::GetStack(const FEntryKey Key) const
{
	return ItemStack.Copies;
}

int32 UFaerieEquipmentSlot::GetStack() const
{
	return ItemStack.Copies;
}
//~ UFaerieItemContainerBase

//~ IFaerieItemDataProxy
const UFaerieItem* UFaerieEquipmentSlot::GetItemObject() const
{
	return ItemStack.Item;
}

int32 UFaerieEquipmentSlot::GetCopies() const
{
	return ItemStack.Copies;
}
bool UFaerieEquipmentSlot::CanMutate() const
{
	// Items should not be modified while in an equipment slot. Remove them first.
	return false;
}

TScriptInterface<IFaerieItemOwnerInterface> UFaerieEquipmentSlot::GetOwner() const
{
	return const_cast<ThisClass*>(this);
}
//~ IFaerieItemDataProxy

//~ IFaerieItemOwnerInterface
FFaerieItemStack UFaerieEquipmentSlot::Release(const FFaerieItemStackView Stack)
{
	if (Stack.Item == ItemStack.Item &&
		Stack.Copies >= ItemStack.Copies)
	{
		return TakeItemFromSlot(Stack.Copies);
	}
	return FFaerieItemStack();
}

bool UFaerieEquipmentSlot::Possess(const FFaerieItemStack Stack)
{
	if (CanSetInSlot(Stack))
	{
		SetItemInSlot(Stack);
		return true;
	}

	return false;
}
//~ IFaerieItemOwnerInterface

void UFaerieEquipmentSlot::BroadcastChange()
{
	OnItemChangedNative.Broadcast(this);
	OnItemChanged.Broadcast(this);
}

bool UFaerieEquipmentSlot::CouldSetInSlot(const FFaerieItemStackView View) const
{
	if (!IsValid(View.Item)) return false;

	if (SingleItemSlot)
	{
		if (IsValid(ItemStack.Item))
		{
			return false;
		}
	}

	if (Extensions->AllowsAddition(this, View) == EEventExtensionResponse::Disallowed)
	{
		return false;
	}

	if (IsValid(SlotDescription))
	{
		return SlotDescription->Template->TryMatch(View);
	}
	return false;
}

bool UFaerieEquipmentSlot::CanSetInSlot(const FFaerieItemStackView View) const
{
	if (!IsValid(View.Item)) return false;

	if (IsFilled())
	{
		// Cannot switch items. Remove current first.
		if (View.Item != ItemStack.Item)
		{
			return false;
		}

		if (SingleItemSlot)
		{
			if (IsValid(ItemStack.Item))
			{
				return false;
			}
		}
	}

	if (Extensions->AllowsAddition(this, View) == EEventExtensionResponse::Disallowed)
	{
		return false;
	}

	if (IsValid(SlotDescription))
	{
		return SlotDescription->Template->TryMatch(View);
	}
	return false;
}

bool UFaerieEquipmentSlot::CanTakeFromSlot(const int32 Copies) const
{
	if (!Faerie::ItemData::IsValidStack(Copies) ||
		!IsValid(ItemStack.Item)) return false;

	if (Copies != Faerie::ItemData::UnlimitedStack &&
		ItemStack.Copies < Copies)
	{
		return false;
	}

	if (Extensions->AllowsRemoval(this, StoredKey, FFaerieEquipmentSlotEvents::Get().Take) == EEventExtensionResponse::Disallowed)
	{
		return false;
	}

	return true;
}

void UFaerieEquipmentSlot::SetItemInSlot(const FFaerieItemStack Stack)
{
	if (!CanSetInSlot(Stack))
	{
		UE_LOG(LogFaerieEquipmentSlot, Warning,
			TEXT("Invalid request to set into slot '%s'!"), *GetSlotInfo().ObjectName.ToString())
		return;
	}

	// If the above check passes, then either the Stack's item is the same as our's, or we are currently empty!

	Extensions->PreAddition(this, {ItemStack.Item, 1});

	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, ItemStack, this);
	// Increment key when stored item changes. This is only going to happen if ItemStack.Item is currently nullptr.
	if (Stack.Item != ItemStack.Item)
	{
		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, StoredKey, this);
		StoredKey = NextKey();

		ItemStack = Stack;

		TakeOwnership(ItemStack.Item);
	}
	else
	{
		ItemStack.Copies += Stack.Copies;
		Extensions->PostEntryChanged(this, StoredKey);
	}

	Faerie::Inventory::FEventLog Event;
	Event.Item = ItemStack.Item;
	Event.Amount = 1;
	Event.Success = true;
	Event.Type = FFaerieEquipmentSlotEvents::Get().Set;
	Event.EntryTouched = StoredKey;
	Extensions->PostAddition(this, Event);

	BroadcastChange();
}

FFaerieItemStack UFaerieEquipmentSlot::TakeItemFromSlot(int32 Copies)
{
	if (!CanTakeFromSlot(Copies))
	{
		UE_LOG(LogFaerieEquipmentSlot, Warning,
			TEXT("Invalid request to set into slot '%s'!"), *GetSlotInfo().ObjectName.ToString())
		return FFaerieItemStack();
	}

	if (Copies > ItemStack.Copies)
	{
		UE_LOG(LogFaerieEquipmentSlot, Error,
			TEXT("Cannot remove more copies from a slot than what it contains. Slot: '%s', Requested Copies: '%i', Contained: '%i' !"),
			*GetSlotInfo().ObjectName.ToString(), Copies, ItemStack.Copies)
		return FFaerieItemStack();
	}

	if (Copies == Faerie::ItemData::UnlimitedStack)
	{
		Copies = ItemStack.Copies;
	}

	Extensions->PreRemoval(this, StoredKey, Copies);

	const FFaerieItemStack OutStack{ItemStack.Item, Copies};

	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, ItemStack, this);
	if (Copies == ItemStack.Copies)
	{
		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, StoredKey, this);
		StoredKey = FEntryKey::InvalidKey;

		// Our local Item ptr must be nullptr before calling ReleaseOwnership
		ItemStack = FFaerieItemStack();

		// Take ownership of new items.
		ReleaseOwnership(OutStack.Item);
	}
	else
	{
		ItemStack.Copies -= Copies;
		Extensions->PostEntryChanged(this, StoredKey);
	}

	Faerie::Inventory::FEventLog Event;
	Event.Item = OutStack.Item;
	Event.Amount = OutStack.Copies;
	Event.Success = true;
	Event.Type = FFaerieEquipmentSlotEvents::Get().Take;
	Event.EntryTouched = StoredKey;

	Extensions->PostRemoval(this, Event);

	BroadcastChange();

	return OutStack;
}

FFaerieAssetInfo UFaerieEquipmentSlot::GetSlotInfo() const
{
	if (IsValid(SlotDescription) &&
		IsValid(SlotDescription->Template))
	{
		return SlotDescription->Template->GetDescription();
	}
	return FFaerieAssetInfo();
}

bool UFaerieEquipmentSlot::IsFilled() const
{
	return IsValid(ItemStack.Item) && ItemStack.Copies > 0;
}

void UFaerieEquipmentSlot::OnRep_Item()
{
	BroadcastChange();
}