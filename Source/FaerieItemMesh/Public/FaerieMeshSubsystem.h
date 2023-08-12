// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "AnimUtilityStructs.h"
#include "GameplayTagContainer.h"
#include "FaerieMeshStructs.h"
#include "Subsystems/WorldSubsystem.h"
#include "FaerieMeshSubsystem.generated.h"

class UFaerieMeshTokenBase;
class UDynamicMesh;
class UDynamicMeshPool;
class UFaerieItemDataProxyBase;

UENUM(BlueprintType)
enum class EItemMeshType : uint8
{
	None UMETA(hidden),
	Static,
	Dynamic,
	Skeletal
};

/**
 * An item mesh container. Carries either a static, skeletal, or dynamic mesh, and any materials for them.
 * Usually only one of the three mesh options will be valid.
 */
USTRUCT(BlueprintType)
struct FAERIEITEMMESH_API FFaerieItemMesh
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMesh> StaticMesh = nullptr;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UDynamicMesh> DynamicStaticMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FSkeletonAndAnimClass SkeletonAndAnimClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FFaerieItemMaterial> Materials;

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


/**
 *
 */
USTRUCT()
struct FFaerieCachedMeshKey
{
	GENERATED_BODY()

	UPROPERTY()
	TWeakObjectPtr<UFaerieMeshTokenBase> Token;

	UPROPERTY()
	FGameplayTag Purpose;

	bool IsTokenValid() const
	{
		return Token.IsValid();
	}

	friend bool operator==(const FFaerieCachedMeshKey& Lhs, const FFaerieCachedMeshKey& Rhs)
	{
		return Lhs.Token == Rhs.Token &&
			   Lhs.Purpose == Rhs.Purpose;
	}

	friend bool operator!=(const FFaerieCachedMeshKey& Lhs, const FFaerieCachedMeshKey& Rhs)
	{
		return !(Lhs == Rhs);
	}
};

FORCEINLINE uint32 GetTypeHash(const FFaerieCachedMeshKey& Key)
{
	return HashCombine(GetTypeHash(Key.Token), GetTypeHash(Key.Purpose));
}


/**
 *
 */
UCLASS()
class FAERIEITEMMESH_API UFaerieMeshSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

protected:
	UDynamicMeshPool* GetMeshPool();

public:
	UFUNCTION(BlueprintCallable)
	UDynamicMesh* GetDynamicStaticMeshForData(const FFaerieDynamicStaticMesh& MeshData, TArray<FFaerieItemMaterial>& OutMaterialSet);

	UFUNCTION(BlueprintCallable)
	FSkeletonAndAnimClass GetDynamicSkeletalMeshForData(const FFaerieDynamicSkeletalMesh& MeshData, TArray<FFaerieItemMaterial>& OutMaterialSet);

	// todo make async version of this function? In practice this function hasn't been "that" slow, but for more complex meshes it might be.
	// Immediately retrieves the mesh for an item.
	// WARNING: This can cause a hitch if the mesh is not cached and it requires a lengthy assembly.
	UFUNCTION(BlueprintCallable, Category = "Faerie|MeshSubsystem", Meta = (GameplayTagFilter = "MeshPurpose"))
	UPARAM(DisplayName = "Success") bool LoadMeshFromTokenSynchronous(UFaerieMeshTokenBase* Token, const FGameplayTag Purpose, FFaerieItemMesh& Mesh);

	// todo make async version of this function? In practice this function hasn't been "that" slow, but for more complex meshes it might be.
	// Immediately retrieves the mesh for an item.
	// WARNING: This can cause a hitch if the mesh is not cached and it requires a lengthy assembly.
	UFUNCTION(BlueprintCallable, Category = "Faerie|MeshSubsystem", Meta = (GameplayTagFilter = "MeshPurpose"))
	UPARAM(DisplayName = "Success") bool LoadMeshFromReaderSynchronous(UFaerieItemDataProxyBase* Proxy, const FGameplayTag Purpose, FFaerieItemMesh& Mesh);

protected:
	// If the purpose requested when loading a mesh is not available, the tag "MeshPurpose.Default" is normally used as
	// a fallback. If this is set to a tag other than that, then this will be tried first, before the default.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Meta = (Categories = "MeshPurpose"))
	FGameplayTag FallbackPurpose;

private:
	UPROPERTY(Transient)
	TObjectPtr<UDynamicMeshPool> MeshPool;

	/**
	 * Stored meshes for quick lookup
	 */
	UPROPERTY(Transient)
	TMap<FFaerieCachedMeshKey, FFaerieItemMesh> GeneratedMeshes;
};
