// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "FaerieInventoryComponent.h"

#include "InventoryContentStructsLibrary.h"
#include "Engine/ActorChannel.h"
#include "Net/UnrealNetwork.h"

DEFINE_LOG_CATEGORY(LogFaerieInventoryComponent);

UFaerieInventoryComponent::UFaerieInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
	bReplicateUsingRegisteredSubObjectList = true;
}

void UFaerieInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams SharedParams;
	SharedParams.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, ItemStorage, SharedParams);
}

void UFaerieInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwner()->HasAuthority())
	{
		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, ItemStorage, this);
		ItemStorage = NewObject<UFaerieItemStorage>(this);

		ItemStorage->GetOnKeyAdded().AddUObject(this, &ThisClass::PostEntryAdded);
		ItemStorage->GetOnKeyUpdated().AddUObject(this, &ThisClass::PostEntryChanged);
		ItemStorage->GetOnKeyRemoved().AddUObject(this, &ThisClass::PreEntryRemoved);

		for (auto&& Extension : Extensions)
		{
			ItemStorage->AddExtension(Extension);
		}

		AddReplicatedSubObject(ItemStorage);
	}
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
		for (auto&& Extension : Extensions)
		{
			if (IsValid(Extension))
			{
				AddReplicatedSubObject(Extension);
			}
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

#if WITH_EDITOR

#define LOCTEXT_NAMESPACE "FaerieInventoryComponent_Validation"

EDataValidationResult UFaerieInventoryComponent::IsDataValid(TArray<FText>& ValidationErrors)
{
	for (const TObjectPtr<UInventoryExtensionBase>& Extension : Extensions)
	{
		if (!Extension)
		{
			ValidationErrors.Add(LOCTEXT("InvalidExtension", "An Extension is not assigned correctly!"));
		}
	}

	return Super::IsDataValid(ValidationErrors);
}

#undef LOCTEXT_NAMESPACE

#endif

void UFaerieInventoryComponent::OnRep_ItemStorage()
{
	if (IsValid(ItemStorage))
	{
		ItemStorage->GetOnKeyAdded().AddUObject(this, &ThisClass::PostEntryAdded);
		ItemStorage->GetOnKeyUpdated().AddUObject(this, &ThisClass::PostEntryChanged);
		ItemStorage->GetOnKeyRemoved().AddUObject(this, &ThisClass::PreEntryRemoved);
	}
}

bool UFaerieInventoryComponent::GetExtensionChecked(const TSubclassOf<UInventoryExtensionBase> ExtensionClass,
                                                    UInventoryExtensionBase*& Extension) const
{
	if (!ItemStorage) return false;
	return ItemStorage->GetExtensionChecked(ExtensionClass, Extension);
}

UInventoryExtensionBase* UFaerieInventoryComponent::AddExtension(const TSubclassOf<UInventoryExtensionBase> ExtensionClass)
{
	if (!ensure(IsValid(ExtensionClass)))
	{
		return nullptr;
	}

	if (!ensure(ExtensionClass != UInventoryExtensionBase::StaticClass()))
	{
		return nullptr;
	}

	UInventoryExtensionBase* NewExtension = NewObject<UInventoryExtensionBase>(this, ExtensionClass);
	ItemStorage->AddExtension(NewExtension);
	AddReplicatedSubObject(NewExtension);

	return NewExtension;
}