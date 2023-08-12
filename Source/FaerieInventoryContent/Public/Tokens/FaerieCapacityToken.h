// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "FaerieItemToken.h"
#include "CapacityStructs.h"

#include "FaerieCapacityToken.generated.h"

/**
 *
 */
UCLASS()
class FAERIEINVENTORYCONTENT_API UFaerieCapacityToken : public UFaerieItemToken
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	FItemCapacity GetCapacity() const { return Capacity; }

	UFUNCTION(BlueprintCallable, Category = "CapacityToken")
	int32 GetWeightOfStack(const FInventoryStack Stack) const;

	UFUNCTION(BlueprintCallable, Category = "CapacityToken")
	int32 GetVolumeOfStack(const FInventoryStack Stack) const;

	UFUNCTION(BlueprintCallable, Category = "CapacityToken")
	FWeightAndVolume GetWeightAndVolumeOfStack(const FInventoryStack Stack) const;

	UFUNCTION(BlueprintCallable, Category = "CapacityToken")
	FWeightAndVolume GetWeightAndVolumeOfStackForRemoval(const FInventoryStack Current, const FInventoryStack Removal) const;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Replicated, meta = (ShowOnlyInnerProperties))
	FItemCapacity Capacity;
};
