// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "AnimUtilityStructs.h"
#include "GameplayTagsManager.h"
#include "FaerieMeshStructs.generated.h"

struct FAERIEITEMMESH_API FMeshPurposeTags : public FGameplayTagNativeAdder
{
	FORCEINLINE static const FMeshPurposeTags& Get() { return MeshPurposeTags; }

	FGameplayTag MeshPurposeBase;
	FGameplayTag MP_Default;
	FGameplayTag MP_Display;
	FGameplayTag MP_Equipped;
	FGameplayTag MP_CustomBounds;

protected:
	virtual void AddTags() override
	{
		UGameplayTagsManager& Manager = UGameplayTagsManager::Get();

		MeshPurposeBase = Manager.AddNativeGameplayTag(TEXT("MeshPurpose"), "");

		MP_Default = Manager.AddNativeGameplayTag(TEXT("MeshPurpose.Default"),
												  "");

		MP_Display = Manager.AddNativeGameplayTag(TEXT("MeshPurpose.Display"),
												  "");

		MP_Equipped = Manager.AddNativeGameplayTag(TEXT("MeshPurpose.Equipped"),
												   "Mesh for item when used as active equipment");

		MP_CustomBounds = Manager.AddNativeGameplayTag(TEXT("MeshPurpose.CustomBounds"),
													   "");
	}

private:
	// Private static object for the global tags. Use the Get() function to access externally.
	static FMeshPurposeTags MeshPurposeTags;
};

/**
 * Material reference for a faerie item
 */
USTRUCT(BlueprintType)
struct FAERIEITEMMESH_API FFaerieItemMaterial
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UMaterialInterface> Material = nullptr;

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
};

/**
 * Static mesh source data.
 */
USTRUCT(BlueprintType)
struct FAERIEITEMMESH_API FFaerieStaticMeshData : public FFaerieMeshBaseStruct_NonDynamic
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMesh> StaticMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FFaerieItemMaterial> Materials;
};

/**
 * Skeletal mesh source data.
 */
USTRUCT(BlueprintType)
struct FAERIEITEMMESH_API FFaerieSkeletalMeshData : public FFaerieMeshBaseStruct_NonDynamic
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FSkeletonAndAnimClass SkeletonAndAnimClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FFaerieItemMaterial> Materials;
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

	FString ToString() const;

	static FFaerieDynamicMeshContainer FromString(const FString& String);
};