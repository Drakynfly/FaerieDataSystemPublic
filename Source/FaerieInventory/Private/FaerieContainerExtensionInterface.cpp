// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "FaerieContainerExtensionInterface.h"
#include "ItemContainerExtensionBase.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FaerieContainerExtensionInterface)

bool IFaerieContainerExtensionInterface::RemoveExtensionByClass(const TSubclassOf<UItemContainerExtensionBase> ExtensionClass)
{
	if (!ensure(
			IsValid(ExtensionClass) &&
			ExtensionClass != UItemContainerExtensionBase::StaticClass()))
	{
		return false;
	}

	UItemContainerExtensionBase* Extension = GetExtension(ExtensionClass);
	if (!IsValid(Extension))
	{
		return false;
	}

	return RemoveExtension(Extension);
}

bool IFaerieContainerExtensionInterface::HasExtension(const TSubclassOf<UItemContainerExtensionBase> ExtensionClass) const
{
	return GetExtensionGroup()->HasExtension(ExtensionClass);
}

UItemContainerExtensionBase* IFaerieContainerExtensionInterface::GetExtension(const TSubclassOf<UItemContainerExtensionBase> ExtensionClass) const
{
	return GetExtensionGroup()->GetExtension(ExtensionClass);
}

bool IFaerieContainerExtensionInterface::AddExtension(UItemContainerExtensionBase* Extension)
{
	return GetExtensionGroup()->AddExtension(Extension);
}

UItemContainerExtensionBase* IFaerieContainerExtensionInterface::AddExtensionByClass(const TSubclassOf<UItemContainerExtensionBase> ExtensionClass)
{
	if (!ensure(
		IsValid(ExtensionClass) &&
		ExtensionClass != UItemContainerExtensionBase::StaticClass()))
	{
		return nullptr;
	}

	UItemContainerExtensionBase* NewExtension = NewObject<UItemContainerExtensionBase>(GetExtensionGroup()->_getUObject(), ExtensionClass);
	NewExtension->SetIdentifier();
	AddExtension(NewExtension);

	return NewExtension;
}

bool IFaerieContainerExtensionInterface::RemoveExtension(UItemContainerExtensionBase* Extension)
{
	return GetExtensionGroup()->RemoveExtension(Extension);
}

bool IFaerieContainerExtensionInterface::GetExtensionChecked(const TSubclassOf<UItemContainerExtensionBase> ExtensionClass,
															 UItemContainerExtensionBase*& Extension) const
{
	Extension = GetExtension(ExtensionClass);
	return IsValid(Extension);
}