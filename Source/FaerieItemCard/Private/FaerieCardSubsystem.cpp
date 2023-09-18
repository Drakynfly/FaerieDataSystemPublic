// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "FaerieCardSubsystem.h"
#include "FaerieCardGenerator.h"
#include "FaerieCardSettings.h"

void UFaerieCardSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	auto&& CardSettings = GetDefault<UFaerieCardSettings>();

	Generator = NewObject<UFaerieCardGenerator>(this);
	Generator->DefaultClasses = CardSettings->DefaultClasses;
}

UFaerieCardGenerator* UFaerieCardSubsystem::GetGenerator() const
{
	return Generator;
}