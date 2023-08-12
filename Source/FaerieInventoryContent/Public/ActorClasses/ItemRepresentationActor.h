// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "GameFramework/Actor.h"
#include "ItemRepresentationActor.generated.h"

class UFaerieItemDataProxyBase;

/**
 * The base actor class for physical representations of inventory entries.
 */
UCLASS(Abstract)
class FAERIEINVENTORYCONTENT_API AItemRepresentationActor : public AActor
{
	GENERATED_BODY()

public:
	AItemRepresentationActor();

	UFUNCTION(BlueprintNativeEvent, Category = "Faerie|ItemRepresentationActor")
	void ClearDataDisplay();

	UFUNCTION(BlueprintNativeEvent, Category = "Faerie|ItemRepresentationActor")
	void DisplayData();

private:
	void RegenerateDataDisplay();

public:
	UFUNCTION(BlueprintCallable, Category = "Faerie|ItemRepresentationActor")
	void SetSourceProxy(UFaerieItemDataProxyBase* Source);

protected:
	// The wrapper for the data we are going to display. By keeping the data abstracted behind a UFaerieItemDataProxyBase,
	// this allows AItemRepresentationActor to display data from an Inventory, or standalone data, etc, just as well,
	// with the same API.
	// THIS DOES NOT, AND CANNOT, REPLICATE. If a particular child wants to replicate some or all of the data, it needs
	// to extract out the data it needs into a separate replicated variable. Proxies never replicate.
	UPROPERTY(BlueprintReadOnly, Category = "State")
	TObjectPtr<UFaerieItemDataProxyBase> DataSource = nullptr;
};