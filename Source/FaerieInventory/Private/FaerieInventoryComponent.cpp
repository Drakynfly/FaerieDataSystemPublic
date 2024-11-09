// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "FaerieInventoryComponent.h"
#include "FaerieItemStorage.h"
#include "ItemContainerExtensionBase.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FaerieInventoryComponent)

DEFINE_LOG_CATEGORY(LogFaerieInventoryComponent);

UFaerieInventoryComponent::UFaerieInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
	bReplicateUsingRegisteredSubObjectList = true;

	ItemStorage = CreateDefaultSubobject<UFaerieItemStorage>(FName{TEXTVIEW("ItemStorage")});
	Extensions = CreateDefaultSubobject<UItemContainerExtensionGroup>(FName{TEXTVIEW("Extensions")});
}

void UFaerieInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams SharedParams;
	SharedParams.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, ItemStorage, SharedParams);
}

void UFaerieInventoryComponent::PostInitProperties()
{
	Super::PostInitProperties();

	ItemStorage->GetOnKeyAdded().AddUObject(this, &ThisClass::PostEntryAdded);
	ItemStorage->GetOnKeyUpdated().AddUObject(this, &ThisClass::PostEntryChanged);
	ItemStorage->GetOnKeyRemoved().AddUObject(this, &ThisClass::PreEntryRemoved);
}

void UFaerieInventoryComponent::ReadyForReplication()
{
	Super::ReadyForReplication();

	const AActor* Owner = GetOwner();
	check(Owner);

	if (!Owner->IsUsingRegisteredSubObjectList())
	{
		UE_LOG(LogFaerieInventoryComponent, Warning,
			TEXT("Owner of Inventory Component '%s' does not replicate SubObjectList. Inventory will not be replicated correctly!"), *Owner->GetName())
	}
	else
	{
		AddReplicatedSubObject(ItemStorage);
		AddReplicatedSubObject(Extensions);

		if (IsValid(Extensions))
		{
			ItemStorage->AddExtension(Extensions);
		}
	}
}

void UFaerieInventoryComponent::PostEntryAdded(UFaerieItemStorage* Storage, const FEntryKey Key)
{
#if WITH_EDITOR
	if (GetNetMode() == NM_Client)
	{
		UE_LOG(LogFaerieInventoryComponent, Log, TEXT("Client Received PostContentAdded"))
	}
	else
	{
		UE_LOG(LogFaerieInventoryComponent, Log, TEXT("Server Received PostContentAdded"))
	}
#endif
}

void UFaerieInventoryComponent::PostEntryChanged(UFaerieItemStorage* Storage, const FEntryKey Key)
{
#if WITH_EDITOR
	if (GetNetMode() == NM_Client)
	{
		UE_LOG(LogFaerieInventoryComponent, Log, TEXT("Client Received PostContentChanged"))
	}
	else
	{
		UE_LOG(LogFaerieInventoryComponent, Log, TEXT("Server Received PostContentChanged"))
	}
#endif
}

void UFaerieInventoryComponent::PreEntryRemoved(UFaerieItemStorage* Storage, const FEntryKey Key)
{
#if WITH_EDITOR
	if (GetNetMode() == NM_Client)
	{
		UE_LOG(LogFaerieInventoryComponent, Log, TEXT("Client Received PreContentRemoved"))
	}
	else
	{
		UE_LOG(LogFaerieInventoryComponent, Log, TEXT("Server Received PreContentRemoved"))
	}
#endif
}

bool UFaerieInventoryComponent::GetExtensionChecked(const TSubclassOf<UItemContainerExtensionBase> ExtensionClass,
                                                    UItemContainerExtensionBase*& Extension) const
{
	if (!ItemStorage) return false;
	return ItemStorage->GetExtensionChecked(ExtensionClass, Extension);
}

UItemContainerExtensionBase* UFaerieInventoryComponent::AddExtension(const TSubclassOf<UItemContainerExtensionBase> ExtensionClass)
{
	if (!ensure(IsValid(ExtensionClass)))
	{
		return nullptr;
	}

	if (!ensure(ExtensionClass != UItemContainerExtensionBase::StaticClass()))
	{
		return nullptr;
	}

	UItemContainerExtensionBase* NewExtension = NewObject<UItemContainerExtensionBase>(ItemStorage, ExtensionClass);
	ItemStorage->AddExtension(NewExtension);
	AddReplicatedSubObject(NewExtension);

	return NewExtension;
}