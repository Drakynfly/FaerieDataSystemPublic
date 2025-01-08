// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "AnimUtilityStructs.h"
#include "NativeGameplayTags.h"
#include "FaerieMeshStructs.generated.h"

class UDynamicMesh;

namespace Faerie::ItemMesh::Tags
{
	FAERIEITEMMESH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MeshPurpose_Default)
	FAERIEITEMMESH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MeshPurpose_Display)
	FAERIEITEMMESH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(MeshPurpose_Equipped)
}

/**
 * Material reference for a faerie item
 */
USTRUCT(BlueprintType)
struct FAERIEITEMMESH_API FFaerieItemMaterial
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UMaterialInterface> Material = nullptr;

	friend bool operator==(const FFaerieItemMaterial& Lhs, const FFaerieItemMaterial& Rhs)
	{
		return Lhs.Material == Rhs.Material;
	}

	friend bool operator!=(const FFaerieItemMaterial& Lhs, const FFaerieItemMaterial& Rhs)
	{
		return !(Lhs == Rhs);
	}
};

USTRUCT(BlueprintType)
struct FAERIEITEMMESH_API FFaerieMeshBaseStruct
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (Categories = "MeshPurpose"))
	FGameplayTagContainer Purpose;
};

USTRUCT(BlueprintType)
struct FAERIEITEMMESH_API FFaerieMeshBaseStruct_NonDynamic : public FFaerieMeshBaseStruct
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FFaerieItemMaterial> Materials;
};

/**
 * Static mesh source data.
 */
USTRUCT(BlueprintType)
struct FAERIEITEMMESH_API FFaerieStaticMeshData : public FFaerieMeshBaseStruct_NonDynamic
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UStaticMesh> StaticMesh = nullptr;
};

/**
 * Skeletal mesh source data.
 */
USTRUCT(BlueprintType)
struct FAERIEITEMMESH_API FFaerieSkeletalMeshData : public FFaerieMeshBaseStruct_NonDynamic
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FSoftSkeletonAndAnimClass SkeletonAndAnimClass;
};


USTRUCT(BlueprintType)
struct FAERIEITEMMESH_API FFaerieDynamicStaticMeshFragment : public FFaerieStaticMeshData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FSocketAttachment Attachment;
};

USTRUCT(BlueprintType)
struct FAERIEITEMMESH_API FFaerieDynamicSkeletalMeshFragment : public FFaerieSkeletalMeshData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FSocketAttachment Attachment;
};

USTRUCT(BlueprintType)
struct FAERIEITEMMESH_API FFaerieDynamicStaticMesh : public FFaerieMeshBaseStruct
{
	GENERATED_BODY()

	FFaerieDynamicStaticMesh() {}

	FFaerieDynamicStaticMesh(const FFaerieStaticMeshData& EditorStaticMesh);

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FFaerieDynamicStaticMeshFragment> Fragments;
};

USTRUCT(BlueprintType)
struct FAERIEITEMMESH_API FFaerieDynamicSkeletalMesh : public FFaerieMeshBaseStruct
{
	GENERATED_BODY()

	FFaerieDynamicSkeletalMesh() {}

	FFaerieDynamicSkeletalMesh(const FFaerieSkeletalMeshData& EditorStaticMesh);

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FFaerieDynamicSkeletalMeshFragment> Fragments;
};

FORCEINLINE uint32 GetTypeHash(const FFaerieStaticMeshData& FaerieStaticMeshData)
{
	return FCrc::MemCrc32(&FaerieStaticMeshData, sizeof(FFaerieStaticMeshData));
}

FORCEINLINE uint32 GetTypeHash(const FFaerieSkeletalMeshData& FaerieSkeletalMeshData)
{
	return FCrc::MemCrc32(&FaerieSkeletalMeshData, sizeof(FFaerieSkeletalMeshData));
}

FORCEINLINE uint32 GetTypeHash(const FFaerieDynamicStaticMesh& FaerieDynamicStaticMesh)
{
	return FCrc::MemCrc32(&FaerieDynamicStaticMesh, sizeof(FFaerieDynamicStaticMesh));
}

FORCEINLINE uint32 GetTypeHash(const FFaerieDynamicSkeletalMesh& FaerieDynamicSkeletalMesh)
{
	return FCrc::MemCrc32(&FaerieDynamicSkeletalMesh, sizeof(FFaerieDynamicSkeletalMesh));
}

/**
 * A mesh data container for editor defined static and skeletal meshes.
 */
USTRUCT(BlueprintType)
struct FAERIEITEMMESH_API FFaerieMeshContainer
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Meshes", meta = (Categories = "MeshPurpose"))
	TArray<FFaerieStaticMeshData> StaticMeshes;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Meshes", meta = (Categories = "MeshPurpose"))
	TArray<FFaerieSkeletalMeshData> SkeletalMeshes;

	bool GetStaticItemMesh(const FGameplayTagContainer& SearchPurposes, FFaerieStaticMeshData& Static) const;

	bool GetSkeletalItemMesh(const FGameplayTagContainer& SearchPurposes, FFaerieSkeletalMeshData& Skeletal) const;
};

/**
 * A mesh data container for programatically defined static and skeletal meshes. These are created via crafting and can
 * contain "composable meshes".
 */
USTRUCT(BlueprintType)
struct FAERIEITEMMESH_API FFaerieDynamicMeshContainer
{
	GENERATED_BODY()

	FFaerieDynamicMeshContainer() {}

	// Create a FFaerieDynamicMeshContainer to use in crafting from an asset's FFaerieMeshContainer.
	// All meshes will contain a single fragment, since editor container's only store asset meshes.
	FFaerieDynamicMeshContainer(const FFaerieMeshContainer& EditorContainer)
	{
		for (const FFaerieStaticMeshData& EditorStaticMesh : EditorContainer.StaticMeshes)
		{
			StaticMeshes.Add(EditorStaticMesh);
		}
		for (const FFaerieSkeletalMeshData& EditorSkeletalMesh : EditorContainer.SkeletalMeshes)
		{
			SkeletalMeshes.Add(EditorSkeletalMesh);
		}
	}

	UPROPERTY(BlueprintReadOnly, Category = "Meshes", meta = (Categories = "MeshPurpose"))
	TArray<FFaerieDynamicStaticMesh> StaticMeshes;

	UPROPERTY(BlueprintReadOnly, Category = "Meshes", meta = (Categories = "MeshPurpose"))
	TArray<FFaerieDynamicSkeletalMesh> SkeletalMeshes;
};

/**
 * An item mesh container. Carries either a static, skeletal, or dynamic mesh, and any materials for them.
 * Usually only one of the three mesh options will be valid.
 */
USTRUCT(BlueprintType)
struct FAERIEITEMMESH_API FFaerieItemMesh
{
	GENERATED_BODY()

	static FFaerieItemMesh MakeStatic(UStaticMesh* Mesh, const TArray<FFaerieItemMaterial>& Materials)
	{
		FFaerieItemMesh ItemMesh;
		ItemMesh.StaticMesh = Mesh;
		ItemMesh.Materials = Materials;
		return ItemMesh;
	}

	static FFaerieItemMesh MakeDynamic(UDynamicMesh* Mesh, const TArray<FFaerieItemMaterial>& Materials)
	{
		FFaerieItemMesh ItemMesh;
		ItemMesh.DynamicStaticMesh = Mesh;
		ItemMesh.Materials = Materials;
		return ItemMesh;
	}

	static FFaerieItemMesh MakeSkeletal(const FSkeletonAndAnimClass& Mesh, const TArray<FFaerieItemMaterial>& Materials)
	{
		FFaerieItemMesh ItemMesh;
		ItemMesh.SkeletonAndAnimClass = Mesh;
		ItemMesh.Materials = Materials;
		return ItemMesh;
	}

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMesh> StaticMesh = nullptr;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UDynamicMesh> DynamicStaticMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FSkeletonAndAnimClass SkeletonAndAnimClass;

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FFaerieItemMaterial> Materials;

	bool IsStatic() const;
	bool IsDynamic() const;
	bool IsSkeletal() const;

	UStaticMesh* GetStatic() const
	{
		return StaticMesh;
	}

	UDynamicMesh* GetDynamic() const
	{
		return DynamicStaticMesh;
	}

	FSkeletonAndAnimClass GetSkeletal() const
	{
		return SkeletonAndAnimClass;
	}

	friend bool operator==(const FFaerieItemMesh& Lhs, const FFaerieItemMesh& Rhs)
	{
		return Lhs.StaticMesh == Rhs.StaticMesh
			&& Lhs.DynamicStaticMesh == Rhs.DynamicStaticMesh
			&& Lhs.SkeletonAndAnimClass == Rhs.SkeletonAndAnimClass
			&& Lhs.Materials == Rhs.Materials;
	}

	friend bool operator!=(const FFaerieItemMesh& Lhs, const FFaerieItemMesh& Rhs)
	{
		return !(Lhs == Rhs);
	}
};