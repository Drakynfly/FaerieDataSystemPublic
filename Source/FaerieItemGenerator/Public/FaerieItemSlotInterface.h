// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "UObject/Interface.h"
#include "ItemSlotHandle.h"
#include "StructUtils/StructView.h"

#include "FaerieItemSlotInterface.generated.h"

struct FConstStructView;
class UFaerieItemTemplate;

USTRUCT(BlueprintType)
struct FFaerieItemCraftingSlots
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ItemCraftingSlots")
	TMap<FFaerieItemSlotHandle, TObjectPtr<UFaerieItemTemplate>> RequiredSlots;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ItemCraftingSlots")
	TMap<FFaerieItemSlotHandle, TObjectPtr<UFaerieItemTemplate>> OptionalSlots;
};


UINTERFACE(MinimalAPI, Meta = (CannotImplementInterfaceInBlueprint))
class UFaerieItemSlotInterface : public UInterface
{
	GENERATED_BODY()
};

// @todo 5.5: change to TConstStructView<FFaerieItemCraftingSlots>
using FFaerieCraftingSlotsView = FConstStructView;

/**
 *
 */
class FAERIEITEMGENERATOR_API IFaerieItemSlotInterface
{
	GENERATED_BODY()

public:
	// Returns a struct view of type 'FFaerieItemCraftingSlots'
	virtual FFaerieCraftingSlotsView GetCraftingSlots() const PURE_VIRTUAL(IFaerieItemSlotInterface::GetCraftingSlots, return FFaerieCraftingSlotsView(); )
};

namespace Faerie::Crafting
{
	FAERIEITEMGENERATOR_API FFaerieCraftingSlotsView GetCraftingSlots(const IFaerieItemSlotInterface* Interface);
	FAERIEITEMGENERATOR_API bool IsSlotOptional(const IFaerieItemSlotInterface* Interface, const FFaerieItemSlotHandle& Name);
	FAERIEITEMGENERATOR_API bool FindSlot(const IFaerieItemSlotInterface* Interface, const FFaerieItemSlotHandle& Name, UFaerieItemTemplate*& OutSlot);
}