// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "ActorClasses/ItemRepresentationActor.h"
#include "FaerieItemDataProxy.h"

AItemRepresentationActor::AItemRepresentationActor()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
}

void AItemRepresentationActor::ClearDataDisplay_Implementation()
{
}

void AItemRepresentationActor::DisplayData_Implementation()
{
}

void AItemRepresentationActor::RegenerateDataDisplay()
{
	ClearDataDisplay();

	if (IsValid(DataSource))
	{
		DisplayData();
	}
}

void AItemRepresentationActor::SetSourceProxy(UFaerieItemDataProxyBase* Source)
{
	DataSource = Source;
	RegenerateDataDisplay();
}
