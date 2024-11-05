// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "FaerieCardSubsystem.h"
#include "FaerieCardGenerator.h"
#include "FaerieCardSettings.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FaerieCardSubsystem)

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