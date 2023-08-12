// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "GenerationAction.h"
#include "GenerationAction_CraftItems.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogCraftItems, Log, All);

class UItemCraftingConfig;

UCLASS()
class UGenerationAction_CraftItems : public UGenerationActionWithSlots
{
	GENERATED_BODY()

public:
	struct FActionArgs : Super::FActionArgs
	{
		TObjectPtr<UItemCraftingConfig> CraftConfig = nullptr;
	};

	void Configure(FActionArgs& Args);

protected:
	virtual TArray<FSoftObjectPath> GetAssetsToLoad() const override;
	virtual void Run() override;

private:
	UPROPERTY()
	TObjectPtr<UItemCraftingConfig> Config;
};