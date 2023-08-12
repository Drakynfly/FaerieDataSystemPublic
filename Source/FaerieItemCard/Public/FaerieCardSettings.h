// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "FaerieCardSettings.generated.h"

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

	UPROPERTY(Config, EditAnywhere, Category = "Classes")
	TSoftClassPtr<UFaerieCardBase> DefaultPaletteCardClass;

	UPROPERTY(Config, EditAnywhere, Category = "Classes")
    TSoftClassPtr<UFaerieCardBase> DefaultInfoCardClass;
};
