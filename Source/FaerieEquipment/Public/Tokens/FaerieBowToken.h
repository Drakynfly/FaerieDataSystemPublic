// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "FaerieItemToken.h"

#include "FaerieBowToken.generated.h"

/**
 * Data specifically used to implement bow-type weapons.
 */
UCLASS()
class UFaerieBowToken : public UFaerieItemToken
{
	GENERATED_BODY()

	UFaerieBowToken();

public:
	UPROPERTY(EditInstanceOnly, meta = (ClampMin = 0))
	float DrawStrength = 0.f;
};