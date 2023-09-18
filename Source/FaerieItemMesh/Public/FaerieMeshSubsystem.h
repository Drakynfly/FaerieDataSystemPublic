// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "FaerieItemProxy.h"
#include "GameplayTagContainer.h"
#include "FaerieMeshStructs.h"
#include "Subsystems/WorldSubsystem.h"
#include "FaerieMeshSubsystem.generated.h"

class UFaerieMeshTokenBase;

/**
 *
 */
USTRUCT()
struct FFaerieCachedMeshKey
{
	GENERATED_BODY()

	UPROPERTY()
	TWeakObjectPtr<const UFaerieMeshTokenBase> Token;

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
 * This is a world subsystem that stored dynamically generated meshes for items.
 */
UCLASS()
class FAERIEITEMMESH_API UFaerieMeshSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

public:
	UFUNCTION(BlueprintCallable, Category = "Faerie|MeshSubsystem")
	static FFaerieItemMesh GetDynamicStaticMeshForData(const FFaerieDynamicStaticMesh& MeshData);

	UFUNCTION(BlueprintCallable, Category = "Faerie|MeshSubsystem")
	FFaerieItemMesh GetDynamicSkeletalMeshForData(const FFaerieDynamicSkeletalMesh& MeshData) const;

	// todo make async version of this function? In practice this function hasn't been "that" slow, but for more complex meshes it might be.
	// Immediately retrieves the mesh for an item.
	// WARNING: This can cause a hitch if the mesh is not cached and it requires a lengthy assembly.
	UFUNCTION(BlueprintCallable, Category = "Faerie|MeshSubsystem", Meta = (GameplayTagFilter = "MeshPurpose", ExpandBoolAsExecs = "ReturnValue"))
	bool LoadMeshFromTokenSynchronous(const UFaerieMeshTokenBase* Token, const FGameplayTag Purpose, FFaerieItemMesh& Mesh);

	// todo make async version of this function? In practice this function hasn't been "that" slow, but for more complex meshes it might be.
	// Immediately retrieves the mesh for an item.
	// WARNING: This can cause a hitch if the mesh is not cached and it requires a lengthy assembly.
	UFUNCTION(BlueprintCallable, Category = "Faerie|MeshSubsystem", Meta = (GameplayTagFilter = "MeshPurpose", ExpandBoolAsExecs = "ReturnValue"))
	bool LoadMeshFromProxySynchronous(FFaerieItemProxy Proxy, const FGameplayTag Purpose, FFaerieItemMesh& Mesh);

protected:
	// If the purpose requested when loading a mesh is not available, the tag "MeshPurpose.Default" is normally used as
	// a fallback. If this is set to a tag other than that, then this will be tried first, before the default.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Meta = (Categories = "MeshPurpose"))
	FGameplayTag FallbackPurpose;

private:
	/**
	 * Stored meshes for quick lookup
	 */
	UPROPERTY(Transient)
	TMap<FFaerieCachedMeshKey, FFaerieItemMesh> GeneratedMeshes;
};