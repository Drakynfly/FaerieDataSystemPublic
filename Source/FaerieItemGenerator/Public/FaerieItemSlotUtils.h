// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "UObject/Interface.h"
#include "ItemSlotHandle.h"
#include "StructView.h"

#include "FaerieItemSlotUtils.generated.h"

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

/**
 *
 */
class FAERIEITEMGENERATOR_API IFaerieItemSlotInterface
{
	GENERATED_BODY()

public:
	// Returns a struct view of type 'FFaerieItemCraftingSlots'
	// @todo in the future, it would be nicer to return a templated type, like TConstStructView<FFaerieItemCraftingSlots>, if that exists
	virtual FConstStructView GetCraftingSlots() const PURE_VIRTUAL(IFaerieItemSlotInterface::GetCraftingSlots, return FConstStructView(); )
};

UCLASS()
class FAERIEITEMGENERATOR_API UFaerieItemSlotLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	static FConstStructView GetCraftingSlotsFromObject(const UObject* Object);
	static FConstStructView GetCraftingSlotsFromInterface(const IFaerieItemSlotInterface* Interface);

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Item Creator")
	static void GetCraftingSlots(const TScriptInterface<IFaerieItemSlotInterface> Interface, FFaerieItemCraftingSlots& Slots);

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Item Creator", meta = (DisplayName = "Get Crafting Slots (Message)"))
	static void GetCraftingSlots_Message(UObject* Object, FFaerieItemCraftingSlots& Slots);

	UFUNCTION(BlueprintCallable, Category = "Item Creator")
	static bool IsSlotOptional(const TScriptInterface<IFaerieItemSlotInterface> Interface, const FFaerieItemSlotHandle& Name);

	UFUNCTION(BlueprintCallable, Category = "Item Creator")
	static bool FindSlot(const TScriptInterface<IFaerieItemSlotInterface> Interface, const FFaerieItemSlotHandle& Name, UFaerieItemTemplate*& OutSlot);
};