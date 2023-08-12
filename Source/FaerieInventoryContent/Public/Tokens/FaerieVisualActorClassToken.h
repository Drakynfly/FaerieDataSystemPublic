// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "FaerieItemToken.h"
#include "ActorClasses/ItemRepresentationActor.h"

#include "FaerieVisualActorClassToken.generated.h"

/**
 *
 */
UCLASS(DisplayName = "Token - Visual Actor Class")
class FAERIEINVENTORYCONTENT_API UFaerieVisualActorClassToken : public UFaerieItemToken
{
	GENERATED_BODY()

public:
	const TSoftClassPtr<AItemRepresentationActor>& GetActorClass() const { return ActorClass; }

	TSubclassOf<AItemRepresentationActor> LoadActorClassSynchronous() const;

protected:
	UPROPERTY(EditAnywhere, Category = "VisualActorClassToken")
	TSoftClassPtr<AItemRepresentationActor> ActorClass;
};
