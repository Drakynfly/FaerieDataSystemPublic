// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "GenerationStructsLibrary.h"
#include "CraftingLibrary.generated.h"

class UItemSourcePool;
class UItemGenerationDriver;

DECLARE_LOG_CATEGORY_EXTERN(LogCraftingLibrary, Log, All);

UCLASS()
class FAERIEITEMGENERATOR_API UCraftingLibrary final : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Faerie|Crafting")
	static UItemGenerationDriver* CreateGenerationDriver(UItemSourcePool* Pool, const FGeneratorAmountBase& Amount);
};