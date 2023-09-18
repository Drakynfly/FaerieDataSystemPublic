// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "UObject/Interface.h"
#include "InventoryRedirectorInterface.generated.h"

class UFaerieInventoryComponent;

UINTERFACE(MinimalAPI)
class UInventoryRedirectorInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Used to get pointers to inventory components from actors that may or may not have an inventory, or may themselves
 * redirect to something else that does.
 * E.g, you can call this on a player controller, which will use this interface themselves to get it from the game
 * state. Or on AI controlled pawns that can get it from their AI controller.
 * Or simply when you don't want to cast to something that probably has an inventory.
 */
class FAERIEINVENTORY_API IInventoryRedirectorInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Faerie|InventoryRedirector")
	UFaerieInventoryComponent* Resolve() const;
};