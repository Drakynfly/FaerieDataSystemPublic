// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "InventoryExtensionBase.h"

#include "InventoryContentFilterExtension.generated.h"

class UFaerieItemDataStackViewLiteral;
class UFaerieItemDataFilter;

/**
 * An extension that only allows items matching a filter to be contained.
 */
UCLASS()
class FAERIEINVENTORYCONTENT_API UInventoryContentFilterExtension : public UInventoryExtensionBase
{
	GENERATED_BODY()

public:
	UInventoryContentFilterExtension();

	//~ UInventoryExtensionBase
	virtual EEventExtensionResponse AllowsAddition(const UFaerieItemContainerBase* Container, FFaerieItemStackView Stack) override;
	//~ UInventoryExtensionBase

protected:
	// Filter used to determine if an item can be contained in the inventory
	UPROPERTY(EditAnywhere, Category = "Config", meta = (DisplayThumbnail = false))
	TObjectPtr<UFaerieItemDataFilter> Filter;

private:
	UPROPERTY()
	TObjectPtr<UFaerieItemDataStackViewLiteral> ExecutionContainer;
};