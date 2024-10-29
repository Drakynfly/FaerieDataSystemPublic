// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "CraftingActionConfig.h"
#include "FaerieItemSlotInterface.h"
#include "ItemUpgradeConfig.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogItemUpgradeConfig, Log, All);

/**
 *
 */
UCLASS()
class FAERIEITEMGENERATOR_API UItemUpgradeConfig : public UCraftingActionConfig, public IFaerieItemSlotInterface
{
	GENERATED_BODY()

public:
#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif

	virtual FFaerieCraftingSlotsView GetCraftingSlots() const override;

	// Mutator object, created inline.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Refinement Config")
	TObjectPtr<class UFaerieItemMutator> Mutator;
};

USTRUCT(BlueprintType)
struct FOnTheFlyItemUpgradeConfig
{
	GENERATED_BODY()

	// Options to upgrade or refine this asset.
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Instanced, Category = "Crafting", NoClear)
	TObjectPtr<UItemUpgradeConfig> Config =	nullptr;
};