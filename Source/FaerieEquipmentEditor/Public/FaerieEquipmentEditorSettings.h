// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "Engine/DeveloperSettings.h"
#include "FaerieEquipmentEditorSettings.generated.h"

class UDurabilityGradeAsset;

/**
 *
 */
UCLASS(Config = EditorPerProjectUserSettings, defaultconfig, meta = (DisplayName = "Faerie Data System Editor"))
class FAERIEEQUIPMENTEDITOR_API UFaerieEquipmentEditorSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	virtual FName GetCategoryName() const override;

	TArray<FString> GetDebugInfoForCCM(const float CCM) const;

protected:
	UPROPERTY(EditAnywhere, Config, Category = "Durability Info")
	TArray<TSoftObjectPtr<UDurabilityGradeAsset>> EditorInfoGradeList;
};