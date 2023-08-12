// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "ItemGeneratorConfig.h"

#include "ItemSourcePool.h"
#include "UObject/ObjectSaveContext.h"

DEFINE_LOG_CATEGORY(LogItemGenConfig)

UItemGenerationDriver::UItemGenerationDriver()
{
	Pool = CreateDefaultSubobject<UItemSourcePool>("Pool");
}

void UItemGenerationDriver::PreSave(FObjectPreSaveContext SaveContext)
{
	Super::PreSave(SaveContext);

#if WITH_EDITOR
	if (ensure(IsValid(Pool)))
	{
		Pool->SortTable();
	}
#endif
}

UItemSourcePool* UItemGenerationDriver::GetPool() const
{
	return Pool;
}

FGeneratorAmountBase UItemGenerationDriver::GetAmountResolver() const
{
	return AmountResolver.Get<FGeneratorAmountBase>();
}

FPendingItemGeneration UItemGenerationDriver::Resolve() const
{
	if (!ensure(IsValid(Pool)))
	{
		UE_LOG(LogItemGenConfig, Error, TEXT("Invalid Pool!"));
		return FPendingItemGeneration();
	}

	FPendingItemGeneration Result;

    Result.Drop = Pool->GenerateDrop(Squirrel);
	Result.Count = AmountResolver.Get<FGeneratorAmountBase>().Resolve(Squirrel);
	Result.Squirrel = Squirrel;

	UE_LOG(LogItemGenConfig, Log, TEXT("Chosen Drop: %s"), *Result.Drop.Asset.Object.ToString());

	return Result;
}