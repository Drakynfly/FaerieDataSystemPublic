// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "BenchBehaviorBase.h"
#include "ActorClasses/BenchVisualBase.h"

ABenchVisualBase* UBenchBehaviorBase::GetBenchVisualActor() const
{
	return GetTypedOuter<ABenchVisualBase>();
}

bool UBenchBehaviorBase::CanPlayerInteract() const
{
	if (ExternalInteractionTest.IsBound())
	{
		return ExternalInteractionTest.Execute();
	}

	return DefaultInteractionTest();
}

void UBenchBehaviorBase::SetInteractionTest(const FBenchInteractionTest& InteractionTest)
{
	ExternalInteractionTest = InteractionTest;
}

void UBenchBehaviorBase::NotifyInteractBegin(APlayerController* RequestingPlayer)
{
	BeginInteraction();
	OnInteractionBegin.Broadcast();
}

void UBenchBehaviorBase::NotifyInteractEnd(APlayerController* RequestingPlayer)
{
	EndInteraction();
	OnInteractionEnd.Broadcast();
}