﻿// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "FaerieItemSource.h"
#include "ItemSlotHandle.h"
#include "FaerieItemProxy.h"
#include "ItemInstancingContext_Crafting.generated.h"

class USquirrel;

UCLASS()
class FAERIEITEMGENERATOR_API UItemInstancingContext_Crafting : public UItemInstancingContext
{
	GENERATED_BODY()

public:
	// Used to fill Required & Optional Slots
	UPROPERTY()
	TMap<FFaerieItemSlotHandle, FFaerieItemProxy> InputEntryData;

	// When valid, this context can generate seeded output, otherwise implementations may choose to either fail or
	// resolve to non-seeded output.
	UPROPERTY()
	TObjectPtr<USquirrel> Squirrel = nullptr;
};