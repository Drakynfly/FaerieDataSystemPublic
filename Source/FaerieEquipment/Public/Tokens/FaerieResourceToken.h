﻿// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "FaerieItemToken.h"

#include "FaerieResourceToken.generated.h"

class UDurabilityGradeAsset;

/**
 *
 */
UCLASS()
class FAERIEEQUIPMENT_API UFaerieResourceToken : public UFaerieItemToken
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Resource Set")
	TSoftObjectPtr<UMaterialInterface> Material;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Resource Set")
	TSoftObjectPtr<UDurabilityGradeAsset> Grade;
};