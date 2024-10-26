// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "FaerieEquipmentManager.h"
#include "EquipmentHashing.h"
#include "FaerieEquipmentSlot.h"
#include "FaerieItemStorage.h"
#include "ItemContainerExtensionBase.h"
#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FaerieEquipmentManager)

DEFINE_LOG_CATEGORY(LogEquipmentManager)

UFaerieEquipmentManager::UFaerieEquipmentManager()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
	bReplicateUsingRegisteredSubObjectList = true;
	ExtensionGroup = CreateDefaultSubobject<UItemContainerExtensionGroup>("ExtensionGroup");
}

void UFaerieEquipmentManager::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams Params;
	Params.bIsPushBased = true;
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, Slots, Params);
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, ServerChecksum, Params);
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
		AddReplicatedSubObject(ExtensionGroup);

		for (auto&& Slot : Slots)
		{
			if (IsValid(Slot))
			{
				AddReplicatedSubObject(Slot);
			}
		}
	}
}

void UFaerieEquipmentManager::OnSlotItemChanged(UFaerieEquipmentSlot* Slot)
{
	RecalcLocalChecksum();
	OnEquipmentChangedEvent.Broadcast(Slot);
}

void UFaerieEquipmentManager::RecalcLocalChecksum()
{
	TSet<FFaerieSlotTag> Tags;

	Algo::TransformIf(Slots, Tags,
		[](const TObjectPtr<UFaerieEquipmentSlot> Slot){ return IsValid(Slot); },
		[](const TObjectPtr<UFaerieEquipmentSlot> Slot) { return Slot->SlotID; });

	LocalChecksum = UFaerieEquipmentHashing::HashEquipment(this, Tags, &UFaerieEquipmentHashing::ExecHashEquipmentByName);

	UE_LOG(LogEquipmentManager, Log, TEXT("New LocalChecksum: [%i]"), LocalChecksum.Hash);

	if (GetNetMode() < NM_Client)
	{
		// If we are not a client, update and push the server's checksum
		ServerChecksum = LocalChecksum;
		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, ServerChecksum, this);
	}
	else
	{
		// If we are a client, check if this change has put us into or out of sync.
		CheckLocalChecksum();
	}
}

void UFaerieEquipmentManager::CheckLocalChecksum()
{
	const bool OldChecksumsMatch = ChecksumsMatch;
	ChecksumsMatch = LocalChecksum == ServerChecksum;

	if (OldChecksumsMatch != ChecksumsMatch)
	{
		OnClientChecksumEvent.Broadcast(ChecksumsMatch ?
			EFaerieEquipmentClientChecksumState::Synchronized :
			EFaerieEquipmentClientChecksumState::Desynchronized);
	}
}

void UFaerieEquipmentManager::OnRep_ServerChecksum()
{
	CheckLocalChecksum();
}

UFaerieEquipmentSlot* UFaerieEquipmentManager::AddSlot(const FFaerieSlotTag SlotID, UFaerieEquipmentSlotDescription* Description)
{
	if (!SlotID.IsValid()) return nullptr;
	if (Description == nullptr) return nullptr;

	if (UFaerieEquipmentSlot* NewSlot = NewObject<UFaerieEquipmentSlot>(this);
		ensure(IsValid(NewSlot)))
	{
		NewSlot->SlotID = SlotID;
		NewSlot->SlotDescription = Description;
		NewSlot->SingleItemSlot = true; // @todo expose somewhere
		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, Slots, this)
		Slots.Add(NewSlot);
		AddReplicatedSubObject(NewSlot);

		NewSlot->OnItemChangedNative.AddUObject(this, &ThisClass::OnSlotItemChanged);
		NewSlot->OnItemDataChangedNative.AddUObject(this, &ThisClass::OnSlotItemChanged);

		NewSlot->AddExtension(ExtensionGroup);

		return NewSlot;
	}

	return nullptr;
}

bool UFaerieEquipmentManager::RemoveSlot(UFaerieEquipmentSlot* Slot)
{
	if (IsValid(Slot))
	{
		return false;
	}

	if (Slots.Remove(Slot))
	{
		Slot->RemoveExtension(ExtensionGroup);

		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, Slots, this)
		RemoveReplicatedSubObject(Slot);

		Slot->OnItemChangedNative.RemoveAll(this);
		Slot->OnItemDataChangedNative.RemoveAll(this);

		RecalcLocalChecksum();

		return true;
	}

	return false;
}

UFaerieEquipmentSlot* UFaerieEquipmentManager::FindSlot(const FFaerieSlotTag SlotID, const bool Recursive) const
{
	for (auto&& Slot : Slots)
	{
		if (!IsValid(Slot)) continue;
		if (Slot->SlotID == SlotID)
		{
			return Slot;
		}
	}

	if (Recursive)
	{
		for (auto&& Slot : Slots)
		{
			if (!IsValid(Slot)) continue;
			if (auto&& ChildSlot = Slot->FindSlot(SlotID, true))
			{
				return ChildSlot;
			}
		}
	}

	return nullptr;
}

bool UFaerieEquipmentManager::HasExtension(const TSubclassOf<UItemContainerExtensionBase> ExtensionClass) const
{
	return ExtensionGroup->HasExtension(ExtensionClass);
}

bool UFaerieEquipmentManager::GetExtension(const TSubclassOf<UItemContainerExtensionBase> ExtensionClass,
										   UItemContainerExtensionBase*& Extension) const
{
	Extension = ExtensionGroup->GetExtension(ExtensionClass);
	return IsValid(Extension);
}

UItemContainerExtensionBase* UFaerieEquipmentManager::AddExtension(const TSubclassOf<UItemContainerExtensionBase> ExtensionClass)
{
	if (!ensure(
			IsValid(ExtensionClass) &&
			ExtensionClass != UItemContainerExtensionBase::StaticClass()))
	{
		return nullptr;
	}

	UItemContainerExtensionBase* NewExtension = NewObject<UItemContainerExtensionBase>(this, ExtensionClass);

	ExtensionGroup->AddExtension(NewExtension);
	AddReplicatedSubObject(NewExtension);

	return NewExtension;
}

bool UFaerieEquipmentManager::RemoveExtension(const TSubclassOf<UItemContainerExtensionBase> ExtensionClass)
{
	if (!ensure(
			IsValid(ExtensionClass) &&
			ExtensionClass != UItemContainerExtensionBase::StaticClass()))
	{
		return false;
	}

	UItemContainerExtensionBase* Extension = nullptr;
	GetExtension(ExtensionClass, Extension);
	if (!IsValid(Extension))
	{
		return false;
	}

	ExtensionGroup->RemoveExtension(Extension);
	RemoveReplicatedSubObject(Extension);

	return true;
}

UItemContainerExtensionBase* UFaerieEquipmentManager::AddExtensionToSlot(const FFaerieSlotTag SlotID,
																	 const TSubclassOf<UItemContainerExtensionBase> ExtensionClass)
{
	if (!ensure(
			IsValid(ExtensionClass) &&
			ExtensionClass != UItemContainerExtensionBase::StaticClass()))
	{
		return nullptr;
	}

	auto&& Slot = FindSlot(SlotID, true);
	if (!IsValid(Slot))
	{
		return nullptr;
	}

	UItemContainerExtensionBase* NewExtension = NewObject<UItemContainerExtensionBase>(this, ExtensionClass);

	AddReplicatedSubObject(NewExtension);
	Slot->AddExtension(NewExtension);

	return NewExtension;
}

bool UFaerieEquipmentManager::RemoveExtensionFromSlot(const FFaerieSlotTag SlotID, const TSubclassOf<UItemContainerExtensionBase> ExtensionClass)
{
	if (!ensure(
			IsValid(ExtensionClass) &&
			ExtensionClass != UItemContainerExtensionBase::StaticClass()))
	{
		return false;
	}

	auto&& Slot = FindSlot(SlotID, true);
	if (!IsValid(Slot))
	{
		return false;
	}

	UItemContainerExtensionBase* Extension = Slot->GetExtension(ExtensionClass);
	if (!IsValid(Extension))
	{
		return false;
	}

	RemoveReplicatedSubObject(Extension);
	Slot->RemoveExtension(Extension);

	return true;
}