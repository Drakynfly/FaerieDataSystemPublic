// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "InventoryDataStructs.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "InventoryContentStructsLibrary.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogInventoryLibrary, Log, All);

/**
 * Library for exposing struct and inventory functions to blueprint.
 */
UCLASS()
class FAERIEINVENTORY_API UInventoryContentStructsLibrary final : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// Get the inventory system Empty Stack.
	UFUNCTION(BlueprintPure, Category = "Inventory|Macros")
	static FInventoryStack EmptyStack();

	// Get the inventory system Unlimited Stack.
	UFUNCTION(BlueprintPure, Category = "Inventory|Macros")
	static FInventoryStack UnlimitedStack();

	// Tests if a stack is equal to Unlimited Stack.
	UFUNCTION(BlueprintPure, Category = "Inventory|Macros")
	static bool IsUnlimited(FInventoryStack Stack);

    UFUNCTION(BlueprintPure, Category = "Inventory|Utils")
    static bool IsValid(const FEntryKey Key) { return Key.IsValid(); }

	UFUNCTION(BlueprintPure, Category = "Inventory|Utils")
	static FString ToString(const FEntryKey Key) { return Key.ToString(); }

	UFUNCTION(BlueprintPure, Category = "Inventory|Utils")
	static bool IsValid_InventoryKey(const FInventoryKey Key) { return Key.IsValid(); }

	UFUNCTION(BlueprintPure, Category = "Inventory|Utils")
	static FString ToString_InventoryKey(const FInventoryKey Key) { return Key.ToString(); }

	// Selectively equivalate two entries.
	UFUNCTION(BlueprintPure, Category = "Inventory|Utils", meta = (CompactNodeTitle = "=="))
	static bool Equal_EntryEntry(const FInventoryEntry& A, const FInventoryEntry& B,
		UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/FaerieInventory.EEntryEquivelancyFlags")) int32 Checks);

	// Sort an array of inventory entries by date modified.
	UFUNCTION(BlueprintCallable, Category = "Inventory|Utils")
	static void SortEntriesLastModified(UPARAM(ref)TArray<FInventoryEntry>& Entries);
};