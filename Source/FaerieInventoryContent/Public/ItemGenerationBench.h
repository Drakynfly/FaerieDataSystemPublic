// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "BenchBehaviorBase.h"
#include "ItemGenerationBench.generated.h"

class UItemGenerationDriver;

/**
 *
 */
UCLASS()
class FAERIEINVENTORYCONTENT_API UItemGenerationBench : public UBenchBehaviorBase
{
	GENERATED_BODY()

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

protected:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, NoClear, Category = "ItemGeneration", meta = (NoResetToDefault))
	TArray<TObjectPtr<UItemGenerationDriver>> Drivers;
};
