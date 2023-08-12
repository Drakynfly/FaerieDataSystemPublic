// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "GenerationAction_UpgradeItems.h"
#include "FaerieItemMutator.h"
#include "ItemUpgradeConfig.h"

void UGenerationAction_UpgradeItems::Configure(FActionArgs& Args)
{
	UpgradeConfig = Args.UpgradeConfig;
	ItemBeingUpgraded = Args.ItemBeingUpgraded;
	check(UpgradeConfig);
	check(ItemBeingUpgraded);
	Super::Configure(Args);
}

TArray<FSoftObjectPath> UGenerationAction_UpgradeItems::GetAssetsToLoad() const
{
	TArray<FSoftObjectPath> Paths;

	// Preload any assets that the Mutator wants loaded
	TArray<TSoftObjectPtr<UObject>> RequiredAssets;
	UpgradeConfig->Mutator->GetRequiredAssets(RequiredAssets);

	for (auto&& RequiredAsset : RequiredAssets)
	{
		Paths.Add(RequiredAsset.ToSoftObjectPath());
	}

	return Paths;
}

void UGenerationAction_UpgradeItems::Run()
{
	// Apply the mutator
	if (!UpgradeConfig->Mutator->TryApply(ItemBeingUpgraded))
	{
		return Fail();
	}

	OutProxies.Add(ItemBeingUpgraded);

	if (RunConsumeStep)
	{
		if (UpgradeConfig->Mutator->Implements<UFaerieItemSlotInterface>())
		{
			ConsumeSlotCosts(Cast<IFaerieItemSlotInterface>(UpgradeConfig->Mutator));
		}
	}

	Complete();
}