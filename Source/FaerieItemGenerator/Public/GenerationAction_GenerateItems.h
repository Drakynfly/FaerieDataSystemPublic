// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "GenerationAction.h"
#include "GenerationAction_GenerateItems.generated.h"

struct FPendingItemGeneration;
class UItemGenerationConfig;

/**
 * Contains the asynchronous generation logic for inventory entries.
 * // @todo should be renamed to UCraftingAction_Generate
 */
UCLASS()
class UGenerationAction_GenerateItems : public UCraftingActionBase
{
	GENERATED_BODY()

public:
	struct FActionArgs : Super::FActionArgs
	{
		TArray<TObjectPtr<UItemGenerationConfig>> Drivers;
	};

	void Configure(FActionArgs& Args);

protected:
	virtual TArray<FSoftObjectPath> GetAssetsToLoad() const override;
	virtual void Run() override;

private:
	// Children items to generate.
	TArray<FPendingItemGeneration> PendingGenerations;
};