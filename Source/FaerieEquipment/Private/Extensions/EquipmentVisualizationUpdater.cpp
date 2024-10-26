// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "Extensions/EquipmentVisualizationUpdater.h"
#include "EquipmentVisualizer.h"
#include "FaerieEquipmentManager.h"
#include "FaerieItemContainerBase.h"
#include "ItemContainerEvent.h"
#include "Actors/ItemRepresentationActor.h"
#include "Extensions/RelevantActorsExtension.h"
#include "Extensions/VisualSlotExtension.h"
#include "GameFramework/Character.h"
#include "Tokens/FaerieVisualEquipment.h"

void UEquipmentVisualizationUpdater::InitializeExtension(const UFaerieItemContainerBase* Container)
{
	auto&& Visualizer = GetVisualizer(Container);
	if (!IsValid(Visualizer))
	{
		return;
	}

	Container->ForEachKey(
		[this, Container, Visualizer](const FEntryKey Key)
		{
			CreateNewVisualImpl(Container, Visualizer, Container->Proxy(Key));
			SpawnKeys.Add(Container, Key);
		});
}

void UEquipmentVisualizationUpdater::DeinitializeExtension(const UFaerieItemContainerBase* Container)
{
	TArray<FEntryKey> Keys;
	SpawnKeys.MultiFind(Container, Keys);
	SpawnKeys.Remove(Container);

	if (!Keys.IsEmpty())
	{
		auto&& Visualizer = GetVisualizer(Container);
		if (!IsValid(Visualizer))
		{
			return;
		}

		for (auto&& Key : Keys)
		{
			RemoveOldVisualImpl(Visualizer, Container->Proxy(Key));
		}
	}
}

void UEquipmentVisualizationUpdater::PostAddition(const UFaerieItemContainerBase* Container,
												  const Faerie::Inventory::FEventLog& Event)
{
	// A previously empty slot now has been filled with an item.
	CreateNewVisual(Container, Event.EntryTouched);
}

void UEquipmentVisualizationUpdater::PreRemoval(const UFaerieItemContainerBase* Container, const FEntryKey Key,
	const int32 Removal)
{
	// If the whole stack is being removed, remove the visual for it
	if (Container->GetStack(Key) == Removal || Removal == Faerie::ItemData::UnlimitedStack)
	{
		RemoveOldVisual(Container, Key);
	}
}

void UEquipmentVisualizationUpdater::PostEntryChanged(const UFaerieItemContainerBase* Container, const FEntryKey Key)
{
	// The item in a slot has changed. Recreate the visual.
	// @todo maybe don't always do this?!?! determine if we need to. but how?

	auto&& Visualizer = GetVisualizer(Container);
	if (!IsValid(Visualizer))
	{
		return;
	}
	const FFaerieItemProxy Proxy = Container->Proxy(Key);
	RemoveOldVisualImpl(Visualizer, Proxy);
	CreateNewVisualImpl(Container, Visualizer, Proxy);
}

UEquipmentVisualizer* UEquipmentVisualizationUpdater::GetVisualizer(const UFaerieItemContainerBase* Container)
{
	if (!IsValid(Container))
	{
		return nullptr;
	}

	if (auto&& Relevants = Container->GetExtension<URelevantActorsExtension>())
	{
		if (auto&& Pawn = Relevants->FindActor<APawn>())
		{
			return Pawn->GetComponentByClass<UEquipmentVisualizer>();
		}
	}

	return nullptr;
}

void UEquipmentVisualizationUpdater::CreateNewVisual(const UFaerieItemContainerBase* Container, const FEntryKey Key)
{
	auto&& Visualizer = GetVisualizer(Container);
	if (!IsValid(Visualizer))
	{
		return;
	}

	CreateNewVisualImpl(Container, Visualizer, Container->Proxy(Key));
	SpawnKeys.Add(Container, Key);
}

void UEquipmentVisualizationUpdater::RemoveOldVisual(const UFaerieItemContainerBase* Container, const FEntryKey Key)
{
	auto&& Visualizer = GetVisualizer(Container);
	if (!IsValid(Visualizer))
	{
		return;
	}

	RemoveOldVisualImpl(Visualizer, Container->Proxy(Key));
	SpawnKeys.RemoveSingle(Container, Key);
}

void UEquipmentVisualizationUpdater::CreateNewVisualImpl(const UFaerieItemContainerBase* Container,
	UEquipmentVisualizer* Visualizer, const FFaerieItemProxy Proxy)
{
	if (!Proxy.IsValid())
	{
		return;
	}

	auto&& VisualToken = Proxy->GetItemObject()->GetToken<UFaerieVisualEquipment>();
	if (!IsValid(VisualToken))
	{
		return;
	}

	if (VisualToken->GetActorClass().IsNull())
	{
		return;
	}

	const TSubclassOf<AItemRepresentationActor> VisualClass = VisualToken->GetActorClass().LoadSynchronous();
	if (!IsValid(VisualClass))
	{
		UE_LOG(LogEquipmentManager, Warning, TEXT("VisualClass failed to load!"))
		return;
	}

	auto&& SlotExtension = Container->GetExtension<UVisualSlotExtension>();
	if (!IsValid(SlotExtension))
	{
		UE_LOG(LogEquipmentManager, Warning, TEXT("No slot extension for container!"))
		return;
	}

	FEquipmentVisualAttachment Attachment;
	Attachment.Parent = Visualizer->GetOwner()->FindComponentByTag<USceneComponent>(SlotExtension->GetComponentTag());
	if (!Attachment.Parent.IsValid() && Visualizer->GetOwner()->IsA<ACharacter>())
	{
		// Default to using the character mesh for attachment if no other is found.
		Attachment.Parent = Cast<ACharacter>(Visualizer->GetOwner())->GetMesh();
	}
	Attachment.Socket = SlotExtension->GetSocket();

	AItemRepresentationActor* NewVisual = Visualizer->SpawnVisualActorNative<AItemRepresentationActor>(
		{ Proxy }, VisualClass, Attachment);
	if (!IsValid(NewVisual))
	{
		return;
	}

	NewVisual->SetSourceProxy(Proxy);
}

void UEquipmentVisualizationUpdater::RemoveOldVisualImpl(UEquipmentVisualizer* Visualizer, const FFaerieItemProxy Proxy)
{
	check(Visualizer);

	if (AItemRepresentationActor* Visual = Cast<AItemRepresentationActor>(Visualizer->GetSpawnedActorByKey({ Proxy })))
	{
		Visual->ClearDataDisplay();
	}

	Visualizer->DestroyVisualActor({ Proxy });
}