// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "Subsystems/LocalPlayerSubsystem.h"
#include "FaerieCardGeneratorInterface.h"

#include "FaerieCardGenerator.h"

#include "FaerieCardSubsystem.generated.h"

class UFaerieCardBase;

/**
 * A per-player subsystem responsible for generating widget "cards" to display a Faerie Item
 */
UCLASS()
class FAERIEITEMCARD_API UFaerieCardSubsystem : public ULocalPlayerSubsystem, public IFaerieCardGeneratorInterface
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	virtual UFaerieCardGenerator* GetGenerator() const override;

public:
	UFUNCTION(BlueprintCallable, Category = "Faerie|CardSubsystem")
	void GeneratePaletteCard(UFaerieItemDataProxyBase* ItemData, const FFaerieCardGenerationResultDynamic& Callback);

	UFUNCTION(BlueprintCallable, Category = "Faerie|CardSubsystem")
	void GenerateFullCard(UFaerieItemDataProxyBase* ItemData, const FFaerieCardGenerationResultDynamic& Callback);

private:
	UPROPERTY()
	TObjectPtr<UFaerieCardGenerator> Generator;
};