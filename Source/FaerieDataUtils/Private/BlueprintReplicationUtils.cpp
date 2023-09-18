// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "BlueprintReplicationUtils.h"

bool UBlueprintReplicationUtils::AddReplicatedSubObject(AActor* Actor, UObject* Object)
{
	if (Object && Actor)
	{
		if (Object->GetTypedOuter<AActor>() != Actor)
		{
			UE_LOG(LogTemp, Warning,
				TEXT("AddObjectToActorReplicateSubObjectList: Should not register Object to Actor that does not own it."
						" GivenActor: '%s', Object: '%s' DirectOuter: '%s', FirstActorOuter: '%s'"),
						*Actor->GetName(), *Object->GetName(), *Object->GetOuter()->GetName(),
						*Object->GetTypedOuter<AActor>()->GetName())
			return false;
		}

		Actor->AddReplicatedSubObject(Object);
		return Actor->IsReplicatedSubObjectRegistered(Object);
	}
	return false;
}