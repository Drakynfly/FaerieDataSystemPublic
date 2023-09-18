// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "GenerationAction.h"
#include "GenerationAction_UpgradeItems.generated.h"

class UItemUpgradeConfig;

// @todo should be renamed to UCraftingAction_Upgrade
UCLASS()
class UGenerationAction_UpgradeItems : public UCraftingActionWithSlots
{
	GENERATED_BODY()

public:
	struct FActionArgs : Super::FActionArgs
	{
		TScriptInterface<IFaerieItemDataProxy> ItemBeingUpgraded = nullptr;
		TObjectPtr<UItemUpgradeConfig> UpgradeConfig = nullptr;
	};

	void Configure(FActionArgs& Args);

protected:
	virtual TArray<FSoftObjectPath> GetAssetsToLoad() const override;
	virtual void Run() override;

private:
	UPROPERTY()
	TScriptInterface<IFaerieItemDataProxy> ItemBeingUpgraded = nullptr;

	UPROPERTY()
	TObjectPtr<UItemUpgradeConfig> UpgradeConfig = nullptr;
};