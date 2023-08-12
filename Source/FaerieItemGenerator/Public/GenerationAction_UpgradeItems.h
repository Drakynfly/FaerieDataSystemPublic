// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "GenerationAction.h"
#include "GenerationAction_UpgradeItems.generated.h"

class UItemUpgradeConfig;

UCLASS()
class UGenerationAction_UpgradeItems : public UGenerationActionWithSlots
{
	GENERATED_BODY()

public:
	struct FActionArgs : Super::FActionArgs
	{
		TObjectPtr<UFaerieItemDataProxyBase> ItemBeingUpgraded = nullptr;
		TObjectPtr<UItemUpgradeConfig> UpgradeConfig = nullptr;
	};

	void Configure(FActionArgs& Args);

protected:
	virtual TArray<FSoftObjectPath> GetAssetsToLoad() const override;
	virtual void Run() override;

private:
	UPROPERTY()
	TObjectPtr<UFaerieItemDataProxyBase> ItemBeingUpgraded = nullptr;

	UPROPERTY()
	TObjectPtr<UItemUpgradeConfig> UpgradeConfig = nullptr;
};