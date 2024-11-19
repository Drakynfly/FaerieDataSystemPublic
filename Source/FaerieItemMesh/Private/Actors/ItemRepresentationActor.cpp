// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "Actors/ItemRepresentationActor.h"
#include "FaerieItemDataProxy.h"

AItemRepresentationActor::AItemRepresentationActor()
{
	PrimaryActorTick.bCanEverTick = false;
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

	if (IsValid(DataSource.GetItemObject()))
	{
		DisplayData();
	}
}

void AItemRepresentationActor::SetSourceProxy(const FFaerieItemProxy Source)
{
	DataSource = Source;
	RegenerateDataDisplay();
}