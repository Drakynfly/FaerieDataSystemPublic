// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "Tokens/FaerieVisualActorClassToken.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FaerieVisualActorClassToken)

TSubclassOf<AItemRepresentationActor> UFaerieVisualActorClassToken::LoadActorClassSynchronous() const
{
	return ActorClass.LoadSynchronous();
}