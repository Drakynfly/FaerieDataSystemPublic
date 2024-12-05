// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "TTypedTagStaticImpl2.h"
#include "TypedGameplayTags.h"
#include "FaerieSlotTag.generated.h"

/**
 * Tag type to identify equipment slots in a UFaerieEquipmentManager
 */
USTRUCT(BlueprintType, meta = (Categories = "Fae.Slot"))
struct FFaerieSlotTag : public FGameplayTag
{
	GENERATED_BODY()
	END_TAG_DECL2(FFaerieSlotTag, TEXT("Fae.Slot"))
};

namespace Faerie::Equipment::Tags
{
	// Basic equipment slots @todo move these out of plugin to demo project
	FAERIEEQUIPMENT_API UE_DECLARE_GAMEPLAY_TAG_TYPED_EXTERN(FFaerieSlotTag, SlotBody)
	FAERIEEQUIPMENT_API UE_DECLARE_GAMEPLAY_TAG_TYPED_EXTERN(FFaerieSlotTag, SlotHandRight)
	FAERIEEQUIPMENT_API UE_DECLARE_GAMEPLAY_TAG_TYPED_EXTERN(FFaerieSlotTag, SlotHandLeft)

	// Default slots, eg, for a hot-bar.
	FAERIEEQUIPMENT_API UE_DECLARE_GAMEPLAY_TAG_TYPED_EXTERN(FFaerieSlotTag, Slot1)
	FAERIEEQUIPMENT_API UE_DECLARE_GAMEPLAY_TAG_TYPED_EXTERN(FFaerieSlotTag, Slot2)
	FAERIEEQUIPMENT_API UE_DECLARE_GAMEPLAY_TAG_TYPED_EXTERN(FFaerieSlotTag, Slot3)
}