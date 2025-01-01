// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "FaerieItemToken.h"
#include "FaerieDurabilityToken.generated.h"

class UDurabilityGradeAsset;

// @todo remove or merge with ResourceToken

/**
 *
 */
UCLASS(DisplayName = "Token - Durability Grade (DEPRECATED)")
class FAERIEEQUIPMENT_API UFaerieDurabilityToken : public UFaerieItemToken
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Durability Set")
	TSoftObjectPtr<UDurabilityGradeAsset> Grade;
};