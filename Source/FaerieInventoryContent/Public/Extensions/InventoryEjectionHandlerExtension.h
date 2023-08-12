// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "InventoryExtensionBase.h"
#include "FaerieItemStack.h"
#include "Engine/StreamableManager.h"

#include "InventoryEjectionHandlerExtension.generated.h"

struct FAERIEINVENTORYCONTENT_API FFaerieEjectionEvent : public FGameplayTagNativeAdder
{
	FORCEINLINE static const FFaerieEjectionEvent& Get() { return FaerieEjectionEvent; }

	FFaerieInventoryTag Removal_Ejection;

protected:
	virtual void AddTags() override
	{
		Removal_Ejection = FFaerieInventoryTag::AddNativeTag(TEXT("Removal.Ejection"),
						"Remove an item and eject it from the inventory as a pickup/visual");
	}

private:
	// Private static object for the global tags. Use the Get() function to access externally.
	static FFaerieEjectionEvent FaerieEjectionEvent;
};

class AItemRepresentationActor;

/**
 * An inventory extension that allows items to be removed from the inventory with the "Ejection" reason, and spawns
 * pickups for them.
 */
UCLASS()
class FAERIEINVENTORYCONTENT_API UInventoryEjectionHandlerExtension : public UInventoryExtensionBase
{
	GENERATED_BODY()

public:
	//~ UInventoryExtensionBase
	virtual EEventExtensionResponse AllowsRemoval(const UFaerieItemContainerBase* Container, const FEntryKey Key, const FFaerieInventoryTag Reason) const override;
	virtual void PostRemoval(const UFaerieItemContainerBase* Container, const Faerie::FItemContainerEvent& Event) override;
	//~ UInventoryExtensionBase

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
	FStreamableManager StreamableManager;
};
