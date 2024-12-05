// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "Actions/FaerieInventoryClient.h"
#include "GridExtensionClientActions.generated.h"

USTRUCT(BlueprintType)
struct FFaerieClientAction_RequestMoveItemBetweenSpatialSlots : public FFaerieClientActionBase
{
	GENERATED_BODY()

	virtual bool Server_Execute(const UFaerieInventoryClient* Client) const override;

	UPROPERTY(BlueprintReadWrite, Category = "RequestMoveSpatialEntry")
	TObjectPtr<UFaerieItemStorage> Storage = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "RequestMoveSpatialEntry")
	FInventoryKey TargetKey;

	UPROPERTY(BlueprintReadWrite, Category = "RequestMoveSpatialEntry")
	FIntPoint DragEnd = FIntPoint::ZeroValue;
};

USTRUCT(BlueprintType)
struct FFaerieClientAction_RequestRotateSpatialEntry : public FFaerieClientActionBase
{
	GENERATED_BODY()

	virtual bool Server_Execute(const UFaerieInventoryClient* Client) const override;

	UPROPERTY(BlueprintReadWrite, Category = "RequestRotateSpatialEntry")
	TObjectPtr<UFaerieItemStorage> Storage = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "RequestRotateSpatialEntry")
	FInventoryKey Key;

	// @todo should also pass a 90 degree / 180 degree parameter...
};