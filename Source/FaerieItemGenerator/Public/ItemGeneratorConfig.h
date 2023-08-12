// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "GenerationStructsLibrary.h"
#include "GenerationActionConfig.h"
#include "InstancedStruct.h"
#include "ItemGeneratorConfig.generated.h"

class UItemSourcePool;

DECLARE_LOG_CATEGORY_EXTERN(LogItemGenConfig, Log, All);

// A struct containing information about a generated item drop that has not yet been parsed into entries.
USTRUCT(BlueprintType)
struct FPendingItemGeneration
{
	GENERATED_BODY()

	// The drop that was generated.
	UPROPERTY()
	FTableDrop Drop;

	// Initial amount of items dropped. May be split later into multiple entries, if this number is larger than stack
	// limit on the asset.
	UPROPERTY()
	int32 Count = 0;

	// Squirrel pointer given to the Pending Generation. Required when using random generation.
	UPROPERTY()
	TObjectPtr<USquirrel> Squirrel = nullptr;

	bool IsValid() const
	{
		return Drop.IsValid() && Count > 0;
	}
};

/**
 * Configurable item generation wrapper class.
 */
UCLASS()
class FAERIEITEMGENERATOR_API UItemGenerationDriver : public UGenerationActionConfig
{
	GENERATED_BODY()

	friend class UCraftingLibrary;

public:
	UItemGenerationDriver();

	virtual void PreSave(FObjectPreSaveContext SaveContext) override;

#if WITH_EDITOR
	// Editor accessors
	static FName GetMemberName_Pool() { return GET_MEMBER_NAME_CHECKED(ThisClass, Pool); }
#endif

	UFUNCTION(BlueprintCallable, Category = "Faerie|GenerationDriver")
	UItemSourcePool* GetPool() const;

	UFUNCTION(BlueprintCallable, Category = "Faerie|GenerationDriver")
	FGeneratorAmountBase GetAmountResolver() const;

	FPendingItemGeneration Resolve() const;

protected:
	UPROPERTY(Instanced, VisibleInstanceOnly, Category = "Generator", meta = (ShowInnerProperties))
	TObjectPtr<UItemSourcePool> Pool;

	UPROPERTY(EditAnywhere, NoClear, Category = "Generator", meta = (BaseStruct = "/Script/FaerieItemGenerator.GeneratorAmountBase",
				ExcludeBaseStruct, DisplayName = "Amount"))
	FInstancedStruct AmountResolver = FInstancedStruct::Make(FGeneratorAmount_Fixed());
};