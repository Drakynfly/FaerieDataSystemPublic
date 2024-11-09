// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "Engine/DeveloperSettings.h"
#include "FaerieInventorySettings.generated.h"

UENUM()
enum class EFDSContainerOwnershipBehavior : uint8
{
	None,

	// Use Rename to change the outer of items to the container when ownership is taken.
	Rename
};

/**
 *
 */
UCLASS(config = Project, defaultconfig, meta = (DisplayName = "Faerie Inventory"))
class FAERIEINVENTORY_API UFaerieInventorySettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	// UDeveloperSettings implementation
	virtual FName GetCategoryName() const override;
	// End UDeveloperSettings implementation

	// Behavior when containers take ownership of mutable items.
	// If a system expects items to be outer'd to their container, set this to 'Rename'.
	// Usage of the MakeSaveData/LoadSaveData functions' default implementations require this.
	UPROPERTY(EditAnywhere, Config, Category = "Faerie|Inventory")
	EFDSContainerOwnershipBehavior ContainerMutableBehavior = EFDSContainerOwnershipBehavior::None;
};