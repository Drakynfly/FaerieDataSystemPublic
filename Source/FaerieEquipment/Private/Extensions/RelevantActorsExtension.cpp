// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "Extensions/RelevantActorsExtension.h"

AActor* URelevantActorsExtension::FindActor(const TSubclassOf<AActor> Class) const
{
	for (auto&& Actor : RelevantActors)
	{
		if (Actor.IsValid() && Actor->IsA(Class))
		{
			return Actor.Get();
		}
	}
	return nullptr;
}

void URelevantActorsExtension::AddActor(AActor* Actor)
{
	RelevantActors.Add(Actor);
}

void URelevantActorsExtension::RemoveActor(AActor* Actor)
{
	RelevantActors.Remove(Actor);
}