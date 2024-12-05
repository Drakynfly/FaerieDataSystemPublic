// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "Actions/FaerieInventoryClient.h"
#include "GridExtensionClientActions.generated.h"

USTRUCT(BlueprintType)
struct FFaerieClientAction_RequestMoveItemBetweenGridSlots : public FFaerieClientActionBase
{
	GENERATED_BODY()

	virtual bool Server_Execute(const UFaerieInventoryClient* Client) const override;

	UPROPERTY(BlueprintReadWrite, Category = "RequestMoveGridEntry")
	TObjectPtr<UFaerieItemStorage> Storage = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "RequestMoveGridEntry")
	FInventoryKey TargetKey;

	UPROPERTY(BlueprintReadWrite, Category = "RequestMoveGridEntry")
	FIntPoint DragEnd = FIntPoint::ZeroValue;
};

USTRUCT(BlueprintType)
struct FFaerieClientAction_RequestRotateGridEntry : public FFaerieClientActionBase
{
	GENERATED_BODY()

	virtual bool Server_Execute(const UFaerieInventoryClient* Client) const override;

	UPROPERTY(BlueprintReadWrite, Category = "RequestRotateGridEntry")
	TObjectPtr<UFaerieItemStorage> Storage = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "RequestRotateGridEntry")
	FInventoryKey Key;

	// @todo should also pass a 90 degree / 180 degree parameter...
};