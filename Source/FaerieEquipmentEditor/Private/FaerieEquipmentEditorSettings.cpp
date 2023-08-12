// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "..\Public\FaerieEquipmentEditorSettings.h"
#include "DurabilityGradeAsset.h"

FName UFaerieEquipmentEditorSettings::GetCategoryName() const
{
	return FApp::GetProjectName();
}

TArray<FString> UFaerieEquipmentEditorSettings::GetDebugInfoForCCM(const float CCM) const
{
	TArray<FString> InfoStrings;

	for (TSoftObjectPtr<UDurabilityGradeAsset> Grade : EditorInfoGradeList)
	{
		auto&& GradePtr = Grade.LoadSynchronous();

		const float WeightForGrade = GradePtr->DefaultWeightPerCCM * CCM;

		InfoStrings.Add(GradePtr->GetName() + ": " + FString::SanitizeFloat(GradePtr->DefaultWeightPerCCM)
							+ " (" + FString::FromInt(static_cast<int32>(WeightForGrade)) + ")");
	}

	return InfoStrings;
}