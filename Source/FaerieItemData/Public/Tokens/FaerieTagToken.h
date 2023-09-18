// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "FaerieItemToken.h"
#include "GameplayTagContainer.h"
#include "FaerieTagToken.generated.h"


/**
 * A simple token storing Gameplay Tags on an item. Used by some ItemDataFilters to match items
 */
UCLASS(DisplayName = "Token - Tags")
class FAERIEITEMDATA_API UFaerieTagToken : public UFaerieItemToken
{
	GENERATED_BODY()

protected:
	/**
	 * As noted in the parent class, this function rarely needs to be implemented. In this case, gameplay tags might
	 * be being used to distinguish otherwise identical items stacks, even for items with the same name, e.g, giving one
	 * item a "quest"-type tag, should make it stack seperately from others like it.
	 */
	virtual bool CompareWithImpl(const UFaerieItemToken* FaerieItemToken) const override
	{
		return Cast<ThisClass>(FaerieItemToken)->Tags == Tags;
	}

public:
	const FGameplayTagContainer& GetTags() const { return Tags; }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Faerie|TagToken")
	FGameplayTagContainer Tags;
};