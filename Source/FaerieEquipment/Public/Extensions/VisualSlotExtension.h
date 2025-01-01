// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "ItemContainerExtensionBase.h"
#include "VisualSlotExtension.generated.h"

/**
 * A simple token for adding a Slot FName to containers.
 */
UCLASS()
class FAERIEEQUIPMENT_API UVisualSlotExtension : public UItemContainerExtensionBase
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	FName GetSocket() const { return Socket; }
	FName GetComponentTag() const { return ComponentTag; }
	bool GetAllowLeaderPose() const { return AllowLeaderPose; }

	UFUNCTION(BlueprintSetter, Category = "Faerie|VisualSlotExtension")
	void SetSocket(FName InSocket);

	UFUNCTION(BlueprintSetter, Category = "Faerie|VisualSlotExtension")
	void SetComponentTag(FName InComponentTag);

	UFUNCTION(BlueprintSetter, Category = "Faerie|VisualSlotExtension")
	void SetAllowLeaderPose(bool InAllowLeaderPose);

protected:
	UPROPERTY(EditAnywhere, Replicated, BlueprintSetter = "SetSocket", Category = "VisualSlotExtension")
	FName Socket;

	UPROPERTY(EditAnywhere, Replicated, BlueprintSetter = "SetComponentTag", Category = "VisualSlotExtension")
	FName ComponentTag;

	UPROPERTY(EditAnywhere, Replicated, BlueprintSetter = "SetAllowLeaderPose", Category = "VisualSlotExtension")
	bool AllowLeaderPose = true;
};