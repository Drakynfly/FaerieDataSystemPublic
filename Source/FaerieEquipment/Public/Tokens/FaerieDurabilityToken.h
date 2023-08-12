// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "FaerieItemToken.h"
#include "FaerieDurabilityToken.generated.h"

class UDurabilityGradeAsset;

/**
 *
 */
UCLASS()
class FAERIEEQUIPMENT_API UFaerieDurabilityToken : public UFaerieItemToken
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Durability Set")
	TSoftObjectPtr<UDurabilityGradeAsset> Grade;
};
