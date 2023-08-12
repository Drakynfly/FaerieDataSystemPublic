// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "FaerieCardSubsystem.h"

#include "FaerieCardGenerator.h"
#include "FaerieCardSettings.h"
#include "FaerieItemDataProxy.h"

void UFaerieCardSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	auto&& CardSettings = GetDefault<UFaerieCardSettings>();

	Generator = NewObject<UFaerieCardGenerator>(this);
	Generator->DefaultPaletteClass = CardSettings->DefaultPaletteCardClass;
	Generator->DefaultInfoClass = CardSettings->DefaultInfoCardClass;
}

UFaerieCardGenerator* UFaerieCardSubsystem::GetGenerator() const
{
	return Generator;
}

void UFaerieCardSubsystem::GeneratePaletteCard(UFaerieItemDataProxyBase* ItemData, const FFaerieCardGenerationResultDynamic& Callback)
{
	if (!IsValid(Generator)) return;
	Generator->Generate(ItemData, EFaerieCardGeneratorType::Palette, Callback);
}

void UFaerieCardSubsystem::GenerateFullCard(UFaerieItemDataProxyBase* ItemData, const FFaerieCardGenerationResultDynamic& Callback)
{
	if (!IsValid(Generator)) return;
	Generator->Generate(ItemData, EFaerieCardGeneratorType::Info, Callback);
}
