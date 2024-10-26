// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "FaerieEquipmentEditorSettings.h"
#include "DurabilityGradeAsset.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FaerieEquipmentEditorSettings)

FName UFaerieEquipmentEditorSettings::GetCategoryName() const
{
	return FApp::GetProjectName();
}

TArray<FString> UFaerieEquipmentEditorSettings::GetDebugInfoForCCM(const float CCM) const
{
	TArray<FString> InfoStrings;

	for (auto&& Grade : EditorInfoGradeList)
	{
		auto&& GradePtr = Grade.LoadSynchronous();

		const float WeightForGrade = GradePtr->DefaultWeightPerCCM * CCM;

		InfoStrings.Add(GradePtr->GetName() + ": " + FString::SanitizeFloat(GradePtr->DefaultWeightPerCCM)
							+ " (" + FString::FromInt(static_cast<int32>(WeightForGrade)) + ")");
	}

	return InfoStrings;
}