// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "FaerieMeshStructs.h"
#include "UDynamicMesh.h"

namespace Faerie::ItemMesh::Tags
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(MeshPurpose_Default, FName{TEXTVIEW("MeshPurpose.Default")}, "Only mesh, or mesh used as fallback if others fail")
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(MeshPurpose_Display, FName{TEXTVIEW("MeshPurpose.Display")}, "Mesh for visual display, e.g item pickups.")
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(MeshPurpose_Equipped, FName{TEXTVIEW("MeshPurpose.Equipped")}, "Mesh for item when used as active equipment")
}

FFaerieDynamicStaticMesh::FFaerieDynamicStaticMesh(const FFaerieStaticMeshData& EditorStaticMesh)
{
	if (!EditorStaticMesh.StaticMesh.IsValid())
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
	if (!EditorStaticMesh.SkeletonAndAnimClass.Mesh.IsValid())
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
		if (Data.Purpose.HasAnyExact(SearchPurposes))
		{
			Skeletal = Data;
			return true;
		}
	}
	return false;
}

bool FFaerieItemMesh::IsStatic() const
{
	return IsValid(StaticMesh);
}

bool FFaerieItemMesh::IsDynamic() const
{
	return IsValid(DynamicStaticMesh);
}

bool FFaerieItemMesh::IsSkeletal() const
{
	return IsValid(SkeletonAndAnimClass.Mesh);
		//&& IsValid(SkeletonAndAnimClass.AnimClass);
}