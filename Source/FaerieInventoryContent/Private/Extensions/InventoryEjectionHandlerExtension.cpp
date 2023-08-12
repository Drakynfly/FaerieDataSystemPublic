// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "Extensions/InventoryEjectionHandlerExtension.h"

#include "FaerieItemDataProxy.h"
#include "Tokens/FaerieVisualActorClassToken.h"
#include "ActorClasses/ItemRepresentationActor.h"

#include "GameFramework/GameStateBase.h"

FFaerieEjectionEvent FFaerieEjectionEvent::FaerieEjectionEvent;

EEventExtensionResponse UInventoryEjectionHandlerExtension::AllowsRemoval(const UFaerieItemContainerBase* Container, const FEntryKey Key,
                                                                          const FFaerieInventoryTag Reason) const
{
	if (Reason == FFaerieEjectionEvent::Get().Removal_Ejection)
	{
		return EEventExtensionResponse::Allowed;
	}

	return EEventExtensionResponse::NoExplicitResponse;
}

void UInventoryEjectionHandlerExtension::PostRemoval(const UFaerieItemContainerBase* Container, const Faerie::FItemContainerEvent& Event)
{
	// This extension only listens to Ejection removals
	if (Event.Type != FFaerieEjectionEvent::Get().Removal_Ejection) return;

	// Cannot eject null item
	if (!Event.Item.IsValid()) return;

	FFaerieItemStack Stack;
	Stack.Copies = Event.Amount;

	if (Event.Item->IsDataMutable())
	{
		check(Stack.Copies == 1);
	}

	Stack.Item = const_cast<UFaerieItem*>(Event.Item.Get());

	PendingEjectionQueue.Add(Stack);

	if (!IsStreaming)
	{
		HandleNextInQueue();
	}
}

void UInventoryEjectionHandlerExtension::HandleNextInQueue()
{
	if (PendingEjectionQueue.IsEmpty()) return;

	auto&& Stack = PendingEjectionQueue[0];

	TSoftClassPtr<AItemRepresentationActor> ClassToSpawn;

	if (auto&& ClassToken = Stack.Item->GetToken<UFaerieVisualActorClassToken>())
	{
		ClassToSpawn = ClassToken->GetActorClass();
	}
	else
	{
		ClassToSpawn = ExtensionDefaultClass;
	}

	if (ClassToSpawn.IsValid())
	{
		PostLoadClassToSpawn(ClassToSpawn);
	}
	else
	{
		StreamableManager.RequestAsyncLoad(ClassToSpawn.ToSoftObjectPath(),
			FStreamableDelegate::CreateUObject(this, &ThisClass::PostLoadClassToSpawn, ClassToSpawn));
	}
}

void UInventoryEjectionHandlerExtension::PostLoadClassToSpawn(const TSoftClassPtr<AItemRepresentationActor> ClassToSpawn)
{
	const TSubclassOf<AItemRepresentationActor> ActorClass = ClassToSpawn.Get();

	if (!IsValid(ActorClass))
	{
		// Loading the actor class failed. Still remove the pending stack from the queue, tho.
		PendingEjectionQueue.RemoveAt(0);
		return HandleNextInQueue();
	}

	if (!ensure(!PendingEjectionQueue.IsEmpty()))
	{
		return;
	}

	auto&& PendingStack = PendingEjectionQueue[0];
	PendingEjectionQueue.RemoveAt(0);

	FTransform SpawnTransform = IsValid(RelativeSpawningComponent) ? RelativeSpawningComponent->GetComponentTransform() : GetOwner()->GetTransform();
	SpawnTransform = SpawnTransform.GetRelativeTransform(RelativeSpawningTransform);
	FActorSpawnParameters Args;
	Args.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	AItemRepresentationActor* NewPickup = GetOwner()->GetWorld()->SpawnActor<AItemRepresentationActor>(ActorClass, SpawnTransform, Args);

	if (IsValid(NewPickup))
	{
		UFaerieItemDataStackLiteral* FaerieItemStack = NewObject<UFaerieItemDataStackLiteral>(NewPickup);
		FaerieItemStack->SetValue(PendingStack);
		NewPickup->SetSourceProxy(FaerieItemStack);
	}

	HandleNextInQueue();
}
