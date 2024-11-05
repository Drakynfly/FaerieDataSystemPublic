// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "TTypedTagStaticImpl2.h"
#include "TypedGameplayTags.h"
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

namespace Faerie::Equipment::Tags
{
	FAERIEEQUIPMENT_API UE_DECLARE_GAMEPLAY_TAG_TYPED_EXTERN(FFaerieSlotTag, SlotBody)
	FAERIEEQUIPMENT_API UE_DECLARE_GAMEPLAY_TAG_TYPED_EXTERN(FFaerieSlotTag, SlotHandRight)
	FAERIEEQUIPMENT_API UE_DECLARE_GAMEPLAY_TAG_TYPED_EXTERN(FFaerieSlotTag, SlotHandLeft)
}

namespace Faerie::Slot::Tags
{
	FAERIEEQUIPMENT_API UE_DECLARE_GAMEPLAY_TAG_TYPED_EXTERN(FFaerieSlotTag, Slot1)
	FAERIEEQUIPMENT_API UE_DECLARE_GAMEPLAY_TAG_TYPED_EXTERN(FFaerieSlotTag, Slot2)
	FAERIEEQUIPMENT_API UE_DECLARE_GAMEPLAY_TAG_TYPED_EXTERN(FFaerieSlotTag, Slot3)
}