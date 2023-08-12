// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "FaerieMeshStructs.h"

FMeshPurposeTags FMeshPurposeTags::MeshPurposeTags;

FFaerieDynamicStaticMesh::FFaerieDynamicStaticMesh(const FFaerieStaticMeshData& EditorStaticMesh)
{
	if (!IsValid(EditorStaticMesh.StaticMesh))
	{
		UE_LOG(LogTemp, Warning, TEXT("FInventoryDynamicStaticMesh constructed from invalid EditorStaticMesh"))
		return;
	}

	FFaerieDynamicStaticMeshFragment SingleFragment;
	SingleFragment.StaticMesh = EditorStaticMesh.StaticMesh;
	SingleFragment.Materials = EditorStaticMesh.Materials;
	Fragments.Add(SingleFragment);
	Purpose = EditorStaticMesh.Purpose;
}

FFaerieDynamicSkeletalMesh::FFaerieDynamicSkeletalMesh(const FFaerieSkeletalMeshData& EditorStaticMesh)
{
	if (!IsValid(EditorStaticMesh.SkeletonAndAnimClass.Mesh))
	{
		UE_LOG(LogTemp, Warning, TEXT("FInventoryDynamicSkeletalMesh constructed from invalid EditorStaticMesh"))
		return;
	}

	FFaerieDynamicSkeletalMeshFragment SingleFragment;
	SingleFragment.SkeletonAndAnimClass = EditorStaticMesh.SkeletonAndAnimClass;
	SingleFragment.Materials = EditorStaticMesh.Materials;
	Fragments.Add(SingleFragment);
	Purpose = EditorStaticMesh.Purpose;
}

bool FFaerieMeshContainer::GetStaticItemMesh(const FGameplayTagContainer& SearchPurposes,
												FFaerieStaticMeshData& Static) const
{
	for (const FFaerieStaticMeshData& Data : StaticMeshes)
	{
		if (!Data.StaticMesh)
		{
			continue;
		}

		if (Data.Purpose.HasAnyExact(SearchPurposes))
		{
			Static = Data;
			return true;
		}
	}
	return false;
}

bool FFaerieMeshContainer::GetSkeletalItemMesh(const FGameplayTagContainer& SearchPurposes,
												  FFaerieSkeletalMeshData& Skeletal) const
{
	for (const FFaerieSkeletalMeshData& Data : SkeletalMeshes)
	{
		if (!Data.SkeletonAndAnimClass.Mesh
		 || !IsValid(Data.SkeletonAndAnimClass.AnimClass))
		{
			continue;
		}

		if (Data.Purpose.HasAnyExact(SearchPurposes))
		{
			Skeletal = Data;
			return true;
		}
	}
	return false;
}