// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "Internationalization/Text.h"

#include "FaerieAssetInfo.generated.h"

class UTexture2D;

/**
 * A common set of properties for displaying information about an item/asset/thing to the player.
 */
USTRUCT(BlueprintType)
struct FFaerieAssetInfo
{
	GENERATED_BODY()

	// A name or very short description of the object.
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "FaerieInfoObject")
	FText ObjectName;

	// A single line description, such as a "blurb", or subtitle.
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "FaerieInfoObject")
	FText ShortDescription;

	// A potentially long, multi-line description, ala, flavor text.
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "FaerieInfoObject", meta = (Multiline = true))
	FText LongDescription;

	// An item for representing this object.
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "FaerieInfoObject")
	TSoftObjectPtr<UTexture2D> Icon;
};