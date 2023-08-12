// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "FaerieItemToken.h"
#include "FaerieDamageSourceToken.generated.h"

/**
 * Base data required for any weapon-type asset.
 */
UCLASS()
class FAERIEEQUIPMENT_API UFaerieDamageSourceToken : public UFaerieItemToken
{
	GENERATED_BODY()

public:
	UFaerieDamageSourceToken();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Stats", meta = (ClampMin = 0))
	int32 Damage;
};