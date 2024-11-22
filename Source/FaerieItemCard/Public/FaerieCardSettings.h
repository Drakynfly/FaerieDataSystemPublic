// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "Engine/DeveloperSettings.h"
#include "FaerieCardSettings.generated.h"

class UCustomCardClass;
class UFaerieCardBase;

/**
 *
 */
UCLASS(Config = "Project", defaultconfig)
class FAERIEITEMCARD_API UFaerieCardSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	// UDeveloperSettings implementation
	virtual FName GetCategoryName() const override;
	// End UDeveloperSettings implementation

	// Disable to prevent UFaerieCardSubsystem being created
	UPROPERTY(Config, EditAnywhere, Category = "Generator")
	bool CreateCardGeneratorPlayerSubsystems = true;

	UPROPERTY(Config, EditAnywhere, Category = "Classes")
	TMap<TSubclassOf<UCustomCardClass>, TSoftClassPtr<UFaerieCardBase>> DefaultClasses;
};