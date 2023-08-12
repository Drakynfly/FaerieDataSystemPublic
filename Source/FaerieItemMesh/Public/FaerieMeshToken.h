// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "FaerieItemToken.h"
#include "FaerieMeshStructs.h"

#include "FaerieMeshToken.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogMeshToken, Log, All);

/**
 *
 */
UCLASS(Abstract)
class FAERIEITEMMESH_API UFaerieMeshTokenBase : public UFaerieItemToken
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Inventory Asset|Item Info")
	virtual bool GetStaticItemMesh(const FGameplayTagContainer& SearchPurposes, FFaerieStaticMeshData& Static) const { return false; }

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Inventory Asset|Item Info")
	virtual bool GetSkeletalItemMesh(const FGameplayTagContainer& SearchPurposes, FFaerieSkeletalMeshData& Skeletal) const { return false; }

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Inventory Asset|Item Info")
	void GetMeshes(const FGameplayTagContainer& SearchPurposes, bool& FoundStatic, FFaerieStaticMeshData& Static,
				   bool& FoundSkeletal, FFaerieSkeletalMeshData& Skeletal) const;
};


/**
 *
 */
UCLASS(DisplayName = "Token - Mesh")
class FAERIEITEMMESH_API UFaerieMeshToken : public UFaerieMeshTokenBase
{
	GENERATED_BODY()

	friend class UFaerieMeshSubsystem;

public:
#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) override;
#endif

	virtual bool GetStaticItemMesh(const FGameplayTagContainer& SearchPurposes, FFaerieStaticMeshData& Static) const override;
	virtual bool GetSkeletalItemMesh(const FGameplayTagContainer& SearchPurposes, FFaerieSkeletalMeshData& Skeletal) const override;

protected:
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "MeshToken", meta = (ShowOnlyInnerProperties))
	FFaerieMeshContainer MeshContainer;
};

// Dynamic mesh tokens are generated via crafting, they shouldn't be added to items manually, hence the HideDropdown
UCLASS(HideDropdown, DisplayName = "Token - Mesh (Dynamic)")
class UFaerieMeshToken_Dynamic : public UFaerieMeshTokenBase
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MeshToken")
	FFaerieDynamicMeshContainer DynamicMeshContainer;
};