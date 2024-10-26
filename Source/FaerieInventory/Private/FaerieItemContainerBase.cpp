// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "FaerieItemContainerBase.h"

#include "FaerieItemStorage.h"
#include "ItemContainerExtensionBase.h"
#include "Net/UnrealNetwork.h"
#include "Tokens/FaerieItemStorageToken.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FaerieItemContainerBase)

UFaerieItemContainerBase::UFaerieItemContainerBase()
{
	Extensions = CreateDefaultSubobject<UItemContainerExtensionGroup>(FName{TEXTVIEW("Extensions")});
	Extensions->InitializeExtension(this);
}

void UFaerieItemContainerBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams Params;
	Params.bIsPushBased = true;
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, Extensions, Params);
}

FFaerieItemStack UFaerieItemContainerBase::Release(const FFaerieItemStackView Stack)
{
	// This function should be implemented by children.
	unimplemented();
	return FFaerieItemStack();
}

bool UFaerieItemContainerBase::Possess(FFaerieItemStack Stack)
{
	// This function should be implemented by children.
	unimplemented();
	return false;
}

FEntryKey UFaerieItemContainerBase::NextKey()
{
	// Create a new key from each integer in order. Guarantees unique keys generated in a binary searchable order.
	return FEntryKey(NextKeyInt++);
}

void UFaerieItemContainerBase::OnItemMutated(const UFaerieItem* Item, const UFaerieItemToken* Token)
{
}

void UFaerieItemContainerBase::ReleaseOwnership(UFaerieItem* Item)
{
	if (!ensure(IsValid(Item))) return;

	if (Item->IsDataMutable())
	{
		Item->GetNotifyOwnerOfSelfMutation().Unbind();
	}

	// Remove our group of extensions from any sub-storages
	for (auto&& ChildContainers = UFaerieItemContainerToken::GetAllContainersInItem(Item);
		 auto&& ChildContainer : ChildContainers)
	{
		ChildContainer->RemoveExtension(Extensions);
	}
}

void UFaerieItemContainerBase::TakeOwnership(UFaerieItem* Item)
{
	if (!ensure(IsValid(Item))) return;

	if (Item->IsDataMutable())
	{
		Item->GetNotifyOwnerOfSelfMutation().BindUObject(this, &ThisClass::OnItemMutated);
	}

	// Add our group of extensions to any sub-storages
	for (auto&& ChildContainers = UFaerieItemContainerToken::GetAllContainersInItem(Item);
		 auto&& Container : ChildContainers)
	{
		Container->AddExtension(Extensions);
	}
}

bool UFaerieItemContainerBase::AddExtension(UItemContainerExtensionBase* Extension)
{
	return Extensions->AddExtension(Extension);
}

bool UFaerieItemContainerBase::RemoveExtension(UItemContainerExtensionBase* Extension)
{
	return Extensions->RemoveExtension(Extension);
}

bool UFaerieItemContainerBase::HasExtension(const TSubclassOf<UItemContainerExtensionBase> ExtensionClass) const
{
	return Extensions->HasExtension(ExtensionClass);
}

UItemContainerExtensionBase* UFaerieItemContainerBase::GetExtension(const TSubclassOf<UItemContainerExtensionBase> ExtensionClass) const
{
	return Extensions->GetExtension(ExtensionClass);
}

bool UFaerieItemContainerBase::GetExtensionChecked(const TSubclassOf<UItemContainerExtensionBase> ExtensionClass,
													UItemContainerExtensionBase*& Extension) const
{
	return Extensions->GetExtensionChecked(ExtensionClass, Extension);
}