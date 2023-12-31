// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "ActorClasses/BenchVisualBase.h"

#include "BenchBehaviorBase.h"
#include "Net/UnrealNetwork.h"

ABenchVisualBase::ABenchVisualBase()
{
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;
	bReplicateUsingRegisteredSubObjectList = true;
}

void ABenchVisualBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, Behavior);
}

void ABenchVisualBase::BeginPlay()
{
	Super::BeginPlay();

	if (IsValid(Behavior))
	{
		AddReplicatedSubObject(Behavior);
		Behavior->BeginPlay();
	}
}