// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "TTypedTagStaticImpl2.h"
#include "FaerieSlotTag.generated.h"

/**
 * The key used to flag entries with custom data.
 */
USTRUCT(BlueprintType, meta = (Categories = "Fae.Slot"))
struct FFaerieSlotTag : public FGameplayTag
{
	GENERATED_BODY()
	END_TAG_DECL2(FFaerieSlotTag, TEXT("Fae.Slot"))
};

struct FAERIEEQUIPMENT_API FFaerieSlotTags : public FGameplayTagNativeAdder
{
	FORCEINLINE static const FFaerieSlotTags& Get() { return FaerieSlotTags; }

	FFaerieSlotTag Body;
	FFaerieSlotTag Hand_Right;
	FFaerieSlotTag Hand_Left;

protected:
	virtual void AddTags() override
	{
		Body = FFaerieSlotTag::AddNativeTag(TEXT("Body"), "");

		Hand_Right = FFaerieSlotTag::AddNativeTag(TEXT("Hand.Right"), "");

		Hand_Left = FFaerieSlotTag::AddNativeTag(TEXT("Hand.Left"), "");
	}

private:
	// Private static object for the global tags. Use the Get() function to access externally.
	static FFaerieSlotTags FaerieSlotTags;
};