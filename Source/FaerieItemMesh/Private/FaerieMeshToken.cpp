// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "FaerieMeshToken.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

DEFINE_LOG_CATEGORY(LogMeshToken);

void UFaerieMeshTokenBase::GetMeshes(const FGameplayTagContainer& SearchPurposes, bool& FoundStatic,
                                     FFaerieStaticMeshData& Static, bool& FoundSkeletal, FFaerieSkeletalMeshData& Skeletal) const
{
	FoundStatic = GetStaticItemMesh(SearchPurposes, Static);
	FoundSkeletal = GetSkeletalItemMesh(SearchPurposes, Skeletal);
}

#if WITH_EDITOR

#define LOCTEXT_NAMESPACE "FaerieMeshTokenBaseValidation"

EDataValidationResult UFaerieMeshToken::IsDataValid(FDataValidationContext& Context)
{
	FText ErrorMessage;
	bool HasError = false;

	for (auto&& i : MeshContainer.StaticMeshes)
	{
		//for (auto&& Fragment : i.Fragments)
		{
			if (!IsValid(i.StaticMesh))
			{
				ErrorMessage = LOCTEXT("IsDataValid_Failed_InvalidStaticMesh", "Invalid static mesh found");
				Context.AddError(ErrorMessage);
				HasError = true;
			}
		}
	}

	for (auto&& i : MeshContainer.SkeletalMeshes)
	{
		//for (auto&& Fragment : i.Fragments)
		{
			if (!IsValid(i.SkeletonAndAnimClass.Mesh) ||
				!IsValid(i.SkeletonAndAnimClass.AnimClass))
			{
				ErrorMessage = LOCTEXT("IsDataValid_Failed_InvalidSkeletalMesh", "Invalid skeletal mesh found");
				Context.AddError(ErrorMessage);
				HasError = true;
			}
		}
	}

	if (HasError)
	{
		return EDataValidationResult::Invalid;
	}

	return Super::IsDataValid(Context);
}

#undef LOCTEXT_NAMESPACE

#endif

bool UFaerieMeshToken::GetStaticItemMesh(const FGameplayTagContainer& SearchPurposes,
										 FFaerieStaticMeshData& Static) const
{
	return MeshContainer.GetStaticItemMesh(SearchPurposes, Static);
}

bool UFaerieMeshToken::GetSkeletalItemMesh(const FGameplayTagContainer& SearchPurposes,
										   FFaerieSkeletalMeshData& Skeletal) const
{
	return MeshContainer.GetSkeletalItemMesh(SearchPurposes, Skeletal);
}