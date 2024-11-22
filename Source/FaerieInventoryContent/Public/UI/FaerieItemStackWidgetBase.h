// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "Blueprint/IUserObjectListEntry.h"
#include "Blueprint/UserWidget.h"
#include "FaerieItemStackWidgetBase.generated.h"

class UInventoryContentsBase;
class UInventoryStackProxy;

/**
 * Responsible for displaying a single inventory entry in an entry list widget.
 */
UCLASS(Abstract)
class FAERIEINVENTORYCONTENT_API UFaerieItemStackWidgetBase : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void SetInventoryWidget(UInventoryContentsBase* Widget);

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Entry Display")
	TObjectPtr<UInventoryContentsBase> InventoryWidget;

	UPROPERTY(BlueprintReadWrite, Category = "Entry Display")
	TObjectPtr<UInventoryStackProxy> LocalCache;
};