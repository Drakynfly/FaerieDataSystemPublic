// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "FaerieItemSlotUtils.h"
#include "GenerationActionConfig.h"
#include "ItemUpgradeConfig.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogItemUpgradeConfig, Log, All);

/**
 *
 */
UCLASS()
class FAERIEITEMGENERATOR_API UItemUpgradeConfig : public UGenerationActionConfig, public IFaerieItemSlotInterface
{
	GENERATED_BODY()

public:
#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(TArray<FText>& ValidationErrors) override;
#endif

	virtual FConstStructView GetCraftingSlots() const override;

	// Array of mutator objects, created inline.
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