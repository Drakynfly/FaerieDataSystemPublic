// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "ItemContainerExtensionBase.h"
#include "FaerieItemStack.h"
#include "TypedGameplayTags.h"

#include "InventoryEjectionHandlerExtension.generated.h"

namespace Faerie::Inventory::Tags
{
	FAERIEINVENTORYCONTENT_API UE_DECLARE_GAMEPLAY_TAG_TYPED_EXTERN(FFaerieInventoryTag, RemovalEject)
}

class AItemRepresentationActor;

/**
 * An inventory extension that allows items to be removed from the inventory with the "Ejection" reason, and spawns
 * pickups for them.
 */
UCLASS()
class FAERIEINVENTORYCONTENT_API UInventoryEjectionHandlerExtension : public UItemContainerExtensionBase
{
	GENERATED_BODY()

public:
	//~ UItemContainerExtensionBase
	virtual EEventExtensionResponse AllowsRemoval(const UFaerieItemContainerBase* Container, const FEntryKey Key, const FFaerieInventoryTag Reason) const override;
	virtual void PostRemoval(const UFaerieItemContainerBase* Container, const Faerie::Inventory::FEventLog& Event) override;
	//~ UItemContainerExtensionBase

private:
	void HandleNextInQueue();

	void PostLoadClassToSpawn(TSoftClassPtr<AItemRepresentationActor> ClassToSpawn);

protected:
	// Default visual actor when the item has no custom class.
	UPROPERTY(EditAnywhere, Category = "Config")
	TSoftClassPtr<AItemRepresentationActor> ExtensionDefaultClass;

	// Component to get a transform to spawn the actor with.
	UPROPERTY(BlueprintReadWrite, VisibleInstanceOnly, Category = "Config")
	TObjectPtr<USceneComponent> RelativeSpawningComponent;

	// Relative transform to spawn the actor with.
	UPROPERTY(EditAnywhere, Category = "Config")
	FTransform RelativeSpawningTransform;

	UPROPERTY()
	TArray<FFaerieItemStack> PendingEjectionQueue;

private:
	bool IsStreaming = false;
};