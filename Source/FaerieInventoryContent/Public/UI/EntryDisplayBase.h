// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "InventoryUIAction.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "Blueprint/UserWidget.h"
#include "EntryDisplayBase.generated.h"

class UInventoryStackProxy;

/**
 * Responsible for displaying a single inventory entry in an entry list widget.
 */
UCLASS(Abstract)
class FAERIEINVENTORYCONTENT_API UEntryDisplayBase : public UUserWidget, public IUserObjectListEntry
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