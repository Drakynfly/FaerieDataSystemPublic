// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "GenerationAction_CraftItems.h"
#include "FaerieItem.h"
#include "FaerieItemDataProxy.h"
#include "FaerieItemRecipe.h"
#include "FaerieItemSource.h"
#include "ItemCraftingConfig.h"
#include "ItemInstancingContext_Crafting.h"

DEFINE_LOG_CATEGORY(LogCraftItems)

void UGenerationAction_CraftItems::Configure(FActionArgs& Args)
{
	Config = Args.CraftConfig;
	check(Config);
	Super::Configure(Args);
}

TArray<FSoftObjectPath> UGenerationAction_CraftItems::GetAssetsToLoad() const
{
	TArray<FSoftObjectPath> ObjectPaths;

	return ObjectPaths;
}

void UGenerationAction_CraftItems::Run()
{
	UE_LOG(LogCraftItems, Log, TEXT("Running CraftEntries"));

	UItemInstancingContext_Crafting* Context = NewObject<UItemInstancingContext_Crafting>(this);

	Context->Outer = Executor;
	Context->Squirrel = Config->Squirrel;
	Context->InputEntryData = FilledSlots;

	UFaerieItem* NewItem = Config->Recipe->GetItemSource()->CreateItemInstance(Context);
	if (!IsValid(NewItem))
	{
		UE_LOG(LogCraftItems, Error, TEXT("Item Instancing failed for Craft Item!"));
		return Fail();
	}

	auto&& NewStack = NewObject<UFaerieItemDataStackLiteral>();
	NewStack->SetValue(NewItem);

	OutProxies.Add(NewStack);

	if (RunConsumeStep && Config->Recipe->Implements<UFaerieItemSlotInterface>())
	{
		ConsumeSlotCosts(Cast<IFaerieItemSlotInterface>(Config->Recipe));
	}

	Complete();
}