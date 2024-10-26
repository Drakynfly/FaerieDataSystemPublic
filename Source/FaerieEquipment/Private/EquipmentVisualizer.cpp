// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "EquipmentVisualizer.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(EquipmentVisualizer)

UEquipmentVisualizer::UEquipmentVisualizer()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UEquipmentVisualizer::OnComponentDestroyed(const bool bDestroyingHierarchy)
{
	for (auto&& Element : SpawnedActors)
	{
		Element.Value->Destroy();
	}

	for (auto&& Element : SpawnedComponents)
	{
		Element.Value->DestroyComponent();
	}

	Super::OnComponentDestroyed(bDestroyingHierarchy);
}

UObject* UEquipmentVisualizer::GetSpawnedVisualByClass(const TSubclassOf<UObject> Class, FFaerieVisualKey& Key) const
{
	if (!IsValid(Class)) return nullptr;

	UObject* Out = GetSpawnedActorByClass(Class.Get(), Key);

	if (!IsValid(Out))
	{
		Out = GetSpawnedComponentByClass(Class.Get(), Key);
	}

	return Out;
}

AActor* UEquipmentVisualizer::GetSpawnedActorByClass(const TSubclassOf<AActor> Class, FFaerieVisualKey& Key) const
{
	if (!IsValid(Class)) return nullptr;

	for (auto&& SpawnedActor : SpawnedActors)
	{
		if (SpawnedActor.Value && SpawnedActor.Value.IsA(Class))
		{
			Key = SpawnedActor.Key;
			return SpawnedActor.Value;
		}
	}
	return nullptr;
}

USceneComponent* UEquipmentVisualizer::GetSpawnedComponentByClass(const TSubclassOf<USceneComponent> Class, FFaerieVisualKey& Key) const
{
	if (!IsValid(Class)) return nullptr;

	for (auto&& SpawnedComponent : SpawnedComponents)
	{
		if (SpawnedComponent.Value && SpawnedComponent.Value.IsA(Class))
		{
			Key = SpawnedComponent.Key;
			return SpawnedComponent.Value;
		}
	}
	return nullptr;
}

UObject* UEquipmentVisualizer::GetSpawnedVisualByKey(const FFaerieVisualKey Key) const
{
	if (!Key.IsValid()) return nullptr;

	UObject* Out = GetSpawnedActorByKey(Key);

	if (!IsValid(Out))
	{
		Out = GetSpawnedComponentByKey(Key);
	}

	return Out;
}

AActor* UEquipmentVisualizer::GetSpawnedActorByKey(const FFaerieVisualKey Key) const
{
	if (!Key.IsValid()) return nullptr;

	if (auto&& Found = SpawnedActors.Find(Key))
	{
		return *Found;
	}
	return nullptr;
}

USceneComponent* UEquipmentVisualizer::GetSpawnedComponentByKey(const FFaerieVisualKey Key) const
{
	if (!Key.IsValid()) return nullptr;

	if (auto&& Found = SpawnedComponents.Find(Key))
	{
		return *Found;
	}
	return nullptr;
}

AActor* UEquipmentVisualizer::SpawnVisualActor(const FFaerieVisualKey Key, const TSubclassOf<AActor> Class, const FEquipmentVisualAttachment& Attachment)
{
	if (!Key.IsValid()) return nullptr;
	if (!IsValid(Class)) return nullptr;

	if (SpawnedActors.Contains(Key))
	{
		UE_LOG(LogTemp, Error, TEXT("Attempted to spawn a VisualActor using a key already in use!"))
		return nullptr;
	}

	if (UWorld* World = GEngine->GetWorldFromContextObject(GetOwner(), EGetWorldErrorMode::ReturnNull))
	{
		FActorSpawnParameters Params;
		Params.Owner = GetOwner();
		if (AActor* NewActor = World->SpawnActor(Class, &FTransform::Identity, Params))
		{
			NewActor->OnDestroyed.AddDynamic(this, &ThisClass::OnVisualActorDestroyed);

			SpawnedActors.Add(Key, NewActor);
			ReverseMap.Add(NewActor, Key);

			auto TempMetadata = Attachment;

			if (Attachment.Parent.IsValid())
			{
				// @todo these are hardcoded for now.
				TempMetadata.TransformRules.LocationRule = EAttachmentRule::SnapToTarget;
				TempMetadata.TransformRules.RotationRule = EAttachmentRule::SnapToTarget;
				TempMetadata.TransformRules.ScaleRule = EAttachmentRule::KeepRelative;

				NewActor->AttachToComponent(TempMetadata.Parent.Get(), TempMetadata.TransformRules, TempMetadata.Socket);

				KeyedMetadata.FindOrAdd(Key).Attachment = Attachment;
			}

			KeyedMetadata.FindOrAdd(Key).ChangeCallback.ExecuteIfBound(Key, NewActor);
			OnAnyVisualUpdate.Broadcast(Key, NewActor);

			return NewActor;
		}
	}

	return nullptr;
}

bool UEquipmentVisualizer::DestroyVisualActor(const FFaerieVisualKey Key, const bool ClearMetadata)
{
	if (ClearMetadata)
	{
		KeyedMetadata.Remove(Key);
	}

	if (AActor* Visual = GetSpawnedActorByKey(Key))
	{
		// This will clear from SpawnedActors and ReverseMap via OnDestroyed
		Visual->Destroy();
		return true;
	}

	return false;
}

void UEquipmentVisualizer::ResetAttachment(const FFaerieVisualKey Key)
{
	if (AActor* Visual = GetSpawnedActorByKey(Key))
	{
		if (KeyedMetadata.Contains(Key))
		{
			const FEquipmentVisualAttachment& Attachment = KeyedMetadata[Key].Attachment;
			Visual->AttachToComponent(Attachment.Parent.Get(), Attachment.TransformRules, Attachment.Socket);
		}
		else
		{
			Visual->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		}
	}
}

void UEquipmentVisualizer::AwaitOrReceiveUpdate(const FFaerieVisualKey Key, const FEquipmentVisualizerCallback Callback)
{
	// Set callback for any future changes to this key
	KeyedMetadata.FindOrAdd(Key).ChangeCallback = Callback;

	// If a visual by this already exists, execute the callback right now.
	if (auto&& Visual = GetSpawnedVisualByKey(Key))
	{
		Callback.Execute(Key, Visual);
	}
}

FFaerieVisualKey UEquipmentVisualizer::MakeVisualKeyFromProxy(const TScriptInterface<IFaerieItemDataProxy> Proxy)
{
	return { Proxy.GetInterface() };
}

void UEquipmentVisualizer::OnVisualActorDestroyed(AActor* DestroyedActor)
{
	SpawnedActors.Remove(ReverseMap.FindAndRemoveChecked(DestroyedActor));
}