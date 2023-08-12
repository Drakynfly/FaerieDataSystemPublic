// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "Tokens/FaerieVisualActorClassToken.h"

TSubclassOf<AItemRepresentationActor> UFaerieVisualActorClassToken::LoadActorClassSynchronous() const
{
	return ActorClass.LoadSynchronous();
}