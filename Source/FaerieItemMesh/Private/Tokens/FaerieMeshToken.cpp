// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "Tokens/FaerieMeshToken.h"

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

#define LOCTEXT_NAMESPACE "FaerieMeshTokenValidation"

EDataValidationResult UFaerieMeshToken::IsDataValid(FDataValidationContext& Context) const
{
	for (auto&& i : MeshContainer.StaticMeshes)
	{
		if (i.StaticMesh.IsNull())
		{
			Context.AddError(LOCTEXT("IsDataValid_Failed_InvalidStaticMesh", "Invalid static mesh found"));
		}
	}

	for (auto&& i : MeshContainer.SkeletalMeshes)
	{
		if (i.SkeletonAndAnimClass.Mesh.IsNull() ||
			i.SkeletonAndAnimClass.AnimClass.IsNull())
		{
			Context.AddError(LOCTEXT("IsDataValid_Failed_InvalidSkeletalMesh", "Invalid skeletal mesh found"));
		}
	}

	if (Context.GetNumErrors())
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

#if WITH_EDITOR

#define LOCTEXT_NAMESPACE "FaerieMeshToken_DynamicValidation"

EDataValidationResult UFaerieMeshToken_Dynamic::IsDataValid(FDataValidationContext& Context) const
{
	for (auto&& Element : DynamicMeshContainer.StaticMeshes)
	{
		for (auto&& Fragment : Element.Fragments)
		{
			if (Fragment.StaticMesh.IsNull())
			{
				Context.AddError(LOCTEXT("IsDataValid_Failed_InvalidStaticMesh", "Invalid static mesh found"));
			}
		}
	}

	for (auto&& Element : DynamicMeshContainer.SkeletalMeshes)
	{
		for (auto&& Fragment : Element.Fragments)
		{
			if (Fragment.SkeletonAndAnimClass.Mesh.IsNull() ||
				Fragment.SkeletonAndAnimClass.AnimClass.IsNull())
			{
				Context.AddError(LOCTEXT("IsDataValid_Failed_InvalidSkeletalMesh", "Invalid skeletal mesh found"));
			}
		}
	}

	if (Context.GetNumErrors())
	{
		return EDataValidationResult::Invalid;
	}
	return Super::IsDataValid(Context);
}

#undef LOCTEXT_NAMESPACE

#endif