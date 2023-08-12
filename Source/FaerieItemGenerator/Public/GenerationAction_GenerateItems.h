// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "GenerationAction.h"
#include "ItemGeneratorConfig.h"
#include "GenerationAction_GenerateItems.generated.h"

class UItemGenerationDriver;

/**
 * Contains the asynchronous generation logic for inventory entries.
 */
UCLASS()
class UGenerationAction_GenerateItems : public UGenerationActionBase
{
	GENERATED_BODY()

public:
	struct FActionArgs : Super::FActionArgs
	{
		TArray<TObjectPtr<UItemGenerationDriver>> Drivers;
	};

	void Configure(FActionArgs& Args);

protected:
	virtual TArray<FSoftObjectPath> GetAssetsToLoad() const override;
	virtual void Run() override;

private:
	// Children items to generate.
	TArray<FPendingItemGeneration> PendingGenerations;
};