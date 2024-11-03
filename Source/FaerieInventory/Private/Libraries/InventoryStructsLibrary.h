// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "InventoryDataStructs.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "InventoryStructsLibrary.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogInventoryLibrary, Log, All);

/**
 * Library for exposing struct and inventory functions to blueprint.
 */
UCLASS()
class UInventoryStructsLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
    UFUNCTION(BlueprintPure, Category = "Faerie|Inventory|Utils")
    static bool IsValid(const FEntryKey Key) { return Key.IsValid(); }

	UFUNCTION(BlueprintPure, Category = "Faerie|Inventory|Utils")
	static FString ToString(const FEntryKey Key) { return Key.ToString(); }

	UFUNCTION(BlueprintPure, Category = "Faerie|Inventory|Utils")
	static bool IsValid_InventoryKey(const FInventoryKey Key) { return Key.IsValid(); }

	UFUNCTION(BlueprintPure, Category = "Faerie|Inventory|Utils")
	static FString ToString_InventoryKey(const FInventoryKey Key) { return Key.ToString(); }

	// Selectively equivalate two entries.
	UFUNCTION(BlueprintPure, Category = "Faerie|Inventory|Utils", meta = (CompactNodeTitle = "=="))
	static bool Equal_EntryEntry(const FInventoryEntry& A, const FInventoryEntry& B,
		UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/FaerieInventory.EEntryEquivalencyFlags")) int32 Checks);

	// Sort an array of inventory entries by date modified.
	UFUNCTION(BlueprintCallable, Category = "Faerie|Inventory|Utils")
	static void SortEntriesLastModified(UPARAM(ref)TArray<FInventoryEntry>& Entries);

	UFUNCTION(BlueprintPure, Category = "Faerie|Inventory|Utils")
	static int32 GetStackSum(const FInventoryEntry& Entry);

	UFUNCTION(BlueprintPure, Category = "Faerie|Inventory|Utils")
	static FFaerieItemStackView EntryToStackView(const FInventoryEntry& Entry);

	/* Returns true if inventory keys are equal */
	UFUNCTION(BlueprintPure, meta=(DisplayName = "Equal (Inventory Key)", CompactNodeTitle = "==", Keywords = "== equal compare", ScriptOperator = "=="), Category="Inventory Key Struct")
	static bool EqualInventoryKey(FInventoryKey A, FInventoryKey B);

	/* Returns true if inventory keys are equal */
	UFUNCTION(BlueprintPure, meta=(DisplayName = "Equal (Stack Key)", CompactNodeTitle = "==", Keywords = "== equal compare", ScriptOperator = "=="), Category="Stack Key Struct")
	static bool EqualStackyKey(FStackKey A, FStackKey B);
};