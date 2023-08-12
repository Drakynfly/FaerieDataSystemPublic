// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "InventoryExtensionBase.h"
#include "FaerieItemContainerBase.h"
#include "Net/UnrealNetwork.h"

void UInventoryExtensionGroup::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams Params;
	Params.bIsPushBased = true;
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, Extensions, Params);
}

void UInventoryExtensionGroup::InitializeExtension(const UFaerieItemContainerBase* Container)
{
	if (!ensure(IsValid(Container))) return;
	if (Containers.Contains(Container)) return;

	Containers.Add(Container);

	ForEachExtension(Container, [](const UFaerieItemContainerBase* ThisContainer, UInventoryExtensionBase* Extension)
		{
			Extension->InitializeExtension(ThisContainer);
		});
}

void UInventoryExtensionGroup::DeinitializeExtension(const UFaerieItemContainerBase* Container)
{
	if (!ensure(IsValid(Container))) return;
	if (!Containers.Contains(Container)) return;

	Containers.Remove(Container);

	ForEachExtension(Container, [](const UFaerieItemContainerBase* ThisContainer, UInventoryExtensionBase* Extension)
		{
			Extension->DeinitializeExtension(ThisContainer);
		});
}

EEventExtensionResponse UInventoryExtensionGroup::AllowsAddition(const UFaerieItemContainerBase* Container,
                                                                 const FFaerieItemStackView Stack)
{
	EEventExtensionResponse Response = EEventExtensionResponse::NoExplicitResponse;

	// Check each extension, to see if the reason is allowed or denied.
	for (auto&& Extension : Extensions)
	{
		if (!ensure(IsValid(Extension))) continue;

		switch (Extension->AllowsAddition(Container, Stack)) {
		case EEventExtensionResponse::NoExplicitResponse:
			break;
		case EEventExtensionResponse::Allowed:
			// Flag response as allowed, unless another extension bars with a Disallowed
			Response = EEventExtensionResponse::Allowed;
			break;
		case EEventExtensionResponse::Disallowed:
			// Return false immediately if any Extension bars the reason.
			return EEventExtensionResponse::Disallowed;
		default: ;
		}
	}

	return Response;
}

void UInventoryExtensionGroup::PreAddition(const UFaerieItemContainerBase* Container, const FFaerieItemStackView Stack)
{
	ForEachExtension(Container, [Stack](const UFaerieItemContainerBase* ThisContainer, UInventoryExtensionBase* Extension)
		{
			Extension->PreAddition(ThisContainer, Stack);
		});
}

void UInventoryExtensionGroup::PostAddition(const UFaerieItemContainerBase* Container,
                                            const Faerie::FItemContainerEvent& Event)
{
	ForEachExtension(Container, [Event](const UFaerieItemContainerBase* ThisContainer, UInventoryExtensionBase* Extension)
		{
			Extension->PostAddition(ThisContainer, Event);
		});
}

EEventExtensionResponse UInventoryExtensionGroup::AllowsRemoval(const UFaerieItemContainerBase* Container,
                                                                const FEntryKey Key, const FFaerieInventoryTag Reason) const
{
	EEventExtensionResponse Response = EEventExtensionResponse::NoExplicitResponse;

	// Check each extension, to see if the reason is allowed or denied.
	for (auto&& Extension : Extensions)
	{
		if (!ensure(IsValid(Extension))) continue;

		switch (Extension->AllowsRemoval(Container, Key, Reason)) {
		case EEventExtensionResponse::NoExplicitResponse:
			break;
		case EEventExtensionResponse::Allowed:
			// Flag response as allowed, unless another extension bars with a Disallowed
			Response = EEventExtensionResponse::Allowed;
			break;
		case EEventExtensionResponse::Disallowed:
			// Return false immediately if any Extension bars the reason.
			return EEventExtensionResponse::Disallowed;
		default: ;
		}
	}

	return Response;
}

void UInventoryExtensionGroup::PreRemoval(const UFaerieItemContainerBase* Container, const FEntryKey Key, const int32 Removal)
{
	ForEachExtension(Container, [Key, Removal](const UFaerieItemContainerBase* ThisContainer, UInventoryExtensionBase* Extension)
		{
			Extension->PreRemoval(ThisContainer, Key, Removal);
		});
}

void UInventoryExtensionGroup::PostRemoval(const UFaerieItemContainerBase* Container,
                                           const Faerie::FItemContainerEvent& Event)
{
	ForEachExtension(Container, [Event](const UFaerieItemContainerBase* ThisContainer, UInventoryExtensionBase* Extension)
		{
			Extension->PostRemoval(ThisContainer, Event);
		});
}

void UInventoryExtensionGroup::PostEntryChanged(const UFaerieItemContainerBase* Container, const FEntryKey Key)
{
	ForEachExtension(Container, [Key](const UFaerieItemContainerBase* ThisContainer, UInventoryExtensionBase* Extension)
	{
		Extension->PostEntryChanged(ThisContainer, Key);
	});
}

void UInventoryExtensionGroup::ForEachExtension(const UFaerieItemContainerBase* Container, const TFunction<void(const UFaerieItemContainerBase*, UInventoryExtensionBase*)> Func)
{
	for (auto&& Extension : Extensions)
	{
		if (!ensure(IsValid(Extension))) continue;
		Func(Container, Extension);
	}
}

bool UInventoryExtensionGroup::AddExtension(UInventoryExtensionBase* Extension)
{
	if (!ensure(IsValid(Extension)))
	{
		return false;
	}

	// If the extension wants to be unique, fail if one already exists of this class
	if (Extension->IsUnique())
	{
		if (HasExtension(Extension->GetClass()))
		{
			return false;
		}
	}

	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, Extensions, this);
	Extensions.Add(Extension);
	for (auto&& Container : Containers)
	{
		Extension->InitializeExtension(Container);
	}
	return true;
}

bool UInventoryExtensionGroup::RemoveExtension(UInventoryExtensionBase* Extension)
{
	if (!ensure(IsValid(Extension)))
	{
		return false;
	}

	const bool Removed = !!Extensions.Remove(Extension);

	if (Removed)
	{
		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, Extensions, this);
		for (auto&& Container : Containers)
		{
			Extension->DeinitializeExtension(Container);
		}
	}

	return Removed;
}

bool UInventoryExtensionGroup::HasExtension(TSubclassOf<UInventoryExtensionBase> ExtensionClass) const
{
	if (!IsValid(ExtensionClass) || ExtensionClass == UInventoryExtensionBase::StaticClass()) return false;

	return Extensions.FindByPredicate([ExtensionClass](const UInventoryExtensionBase* Ext)
		{
			if (!ensureAlwaysMsgf(IsValid(Ext), TEXT("Extension object is null")))
			{
				return false;
			}

			return Ext->IsA(ExtensionClass);
		}) != nullptr;
}

UInventoryExtensionBase* UInventoryExtensionGroup::GetExtension(const TSubclassOf<UInventoryExtensionBase> ExtensionClass) const
{
	if (!IsValid(ExtensionClass) || ExtensionClass == UInventoryExtensionBase::StaticClass()) return nullptr;

	if (auto&& ExistingExt = Extensions.FindByPredicate([ExtensionClass](const UInventoryExtensionBase* Ext)
		{
			if (!ensureAlwaysMsgf(IsValid(Ext), TEXT("Extension object is null")))
			{
				return false;
			}

			return Ext->IsA(ExtensionClass);
		}))
	{
		return *ExistingExt;
	}

	return nullptr;
}

bool UInventoryExtensionGroup::GetExtensionChecked(const TSubclassOf<UInventoryExtensionBase> ExtensionClass,
													UInventoryExtensionBase*& Extension) const
{
	Extension = GetExtension(ExtensionClass);
	return IsValid(Extension);
}