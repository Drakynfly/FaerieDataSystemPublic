// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "FaerieItemContainerBase.h"

#include "FaerieItemStorage.h"
#include "InventoryExtensionBase.h"
#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"
#include "Tokens/FaerieItemStorageToken.h"

UFaerieItemContainerBase::UFaerieItemContainerBase()
{
	Extensions = CreateDefaultSubobject<UInventoryExtensionGroup>(TEXT("Extensions"));
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

void UFaerieItemContainerBase::ReleaseOwnership(const UFaerieItem* Item)
{
	if (!ensure(IsValid(Item))) return;

	// Remove our group of extensions from any sub-storage
	if (auto&& StorageToken = Item->GetToken<UFaerieItemStorageToken>())
	{
		StorageToken->GetItemStorage()->RemoveExtension(Extensions);
	}
}

void UFaerieItemContainerBase::TakeOwnership(UFaerieItem* Item)
{
	if (!ensure(IsValid(Item))) return;

	// Add our group of extensions to any sub-storage
	if (auto&& StorageToken = Item->GetToken<UFaerieItemStorageToken>())
	{
		StorageToken->GetItemStorage()->AddExtension(Extensions);
	}
}

bool UFaerieItemContainerBase::AddExtension(UInventoryExtensionBase* Extension)
{
	return Extensions->AddExtension(Extension);
}

bool UFaerieItemContainerBase::RemoveExtension(UInventoryExtensionBase* Extension)
{
	return Extensions->RemoveExtension(Extension);
}

bool UFaerieItemContainerBase::HasExtension(const TSubclassOf<UInventoryExtensionBase> ExtensionClass) const
{
	return Extensions->HasExtension(ExtensionClass);
}

UInventoryExtensionBase* UFaerieItemContainerBase::GetExtension(const TSubclassOf<UInventoryExtensionBase> ExtensionClass) const
{
	return Extensions->GetExtension(ExtensionClass);
}

bool UFaerieItemContainerBase::GetExtensionChecked(const TSubclassOf<UInventoryExtensionBase> ExtensionClass,
													UInventoryExtensionBase*& Extension) const
{
	return Extensions->GetExtensionChecked(ExtensionClass, Extension);
}