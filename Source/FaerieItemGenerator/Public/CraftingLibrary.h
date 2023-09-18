// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "GenerationStructsLibrary.h"
#include "CraftingLibrary.generated.h"

struct FFaerieWeightedDropPool;
class UItemGenerationConfig;

DECLARE_LOG_CATEGORY_EXTERN(LogCraftingLibrary, Log, All);

UCLASS()
class FAERIEITEMGENERATOR_API UCraftingLibrary final : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Faerie|Crafting")
	static UItemGenerationConfig* CreateGenerationDriver(const FFaerieWeightedDropPool& Pool, const FGeneratorAmountBase& Amount);
};