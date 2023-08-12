// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "FaerieEquipmentManager.h"
#include "FaerieEquipmentSlot.h"
#include "FaerieItemStorage.h"
#include "InventoryExtensionBase.h"
#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"

UFaerieEquipmentManager::UFaerieEquipmentManager()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
	bReplicateUsingRegisteredSubObjectList = true;

	CachedProxy = CreateDefaultSubobject<UFaerieItemDataStackViewLiteral>("CachedProxy");
}

void UFaerieEquipmentManager::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams Params;
	Params.bIsPushBased = true;
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, Slots, Params);
}

void UFaerieEquipmentManager::ReadyForReplication()
{
	Super::ReadyForReplication();

	const AActor* Owner = GetOwner();
	check(Owner);

	if (!Owner->IsUsingRegisteredSubObjectList())
	{
		UE_LOG(LogTemp, Warning,
			TEXT("Owner of Equipment Manager '%s' does not replicate SubObjectList. Component will not be replicated correctly!"), *Owner->GetName())
	}
	else
	{
		for (auto&& Slot : Slots)
		{
			if (IsValid(Slot))
			{
				AddReplicatedSubObject(Slot);
			}
		}

		for (auto&& Extension : Extensions)
		{
			if (IsValid(Extension))
			{
				AddReplicatedSubObject(Extension);
			}
		}
	}
}

UFaerieEquipmentSlot* UFaerieEquipmentManager::AddSlot(const FFaerieSlotTag SlotID, UFaerieEquipmentSlotDescription* Description)
{
	if (!SlotID.IsValid()) return nullptr;
	if (Description == nullptr) return nullptr;

	UFaerieEquipmentSlot* NewSlot = NewObject<UFaerieEquipmentSlot>(this);
	if (IsValid(NewSlot))
	{
		NewSlot->SlotID = SlotID;
		NewSlot->SlotDescription = Description;
		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, Slots, this)
		Slots.Add(NewSlot);
		AddReplicatedSubObject(NewSlot);

		for (auto&& Extension : Extensions)
		{
			NewSlot->AddExtension(Extension);
		}

		return NewSlot;
	}

	return nullptr;
}

bool UFaerieEquipmentManager::RemoveSlot(UFaerieEquipmentSlot* Slot)
{
	if (Slots.Remove(Slot))
	{
		for (auto&& Extension : Extensions)
		{
			Slot->RemoveExtension(Extension);
		}

		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, Slots, this)
		RemoveReplicatedSubObject(Slot);
		return true;
	}

	return false;
}

UFaerieEquipmentSlot* UFaerieEquipmentManager::FindSlot(const FFaerieSlotTag SlotID) const
{
	for (auto&& Slot : Slots)
	{
		if (!IsValid(Slot)) continue;
		if (Slot->SlotID == SlotID)
		{
			return Slot;
		}
	}

	return nullptr;
}