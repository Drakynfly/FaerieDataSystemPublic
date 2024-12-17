// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "ActorClasses/FaerieDependencyFetcher.h"

UFaerieDependencyFetcher::UFaerieDependencyFetcher()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UFaerieDependencyFetcher::GetDependency_Implementation(const TSubclassOf<UActorComponent> Class,
	UActorComponent*& Component) const
{
	AActor* Owner = GetOwner();
	if (IsValid(Owner))
	{
		Component = Owner->FindComponentByClass(Class);
		if (IsValid(Component))
		{
			return true;
		}
	}

	return false;
}