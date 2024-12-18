// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "Components/SceneComponent.h"

#include "FaerieMeshStructs.h"
#include "ItemMeshType.h"

#include "FaerieItemMeshComponent.generated.h"


UCLASS(ClassGroup = ("Faerie"), meta = (BlueprintSpawnableComponent))
class FAERIEITEMMESH_API UFaerieItemMeshComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UFaerieItemMeshComponent();

	virtual void DestroyComponent(bool bPromoteChildren = false) override;

protected:
	// @todo this will LoadSync the meshes. Make an async version of RebuildMesh and expose a boolean to select between them
	void RebuildMesh();

public:
	UFUNCTION(BlueprintCallable, Category = "Faerie|ItemDataMesh")
	void SetItemMesh(const FFaerieItemMesh& InMeshData);

	UFUNCTION(BlueprintCallable, Category = "Faerie|ItemDataMesh")
	void ClearItemMesh();

	UFUNCTION(BlueprintCallable, Category = "Faerie|ItemDataMesh")
	void SetPreferredMeshType(EItemMeshType MeshType);

	// Get the bound for the current mesh data type.
	UFUNCTION(BlueprintCallable, Category = "Faerie|ItemDataMesh")
	FBoxSphereBounds GetBounds() const;

protected:
	// If MeshData contains this type, it will be used.
	// Otherwise, they will be chosen in the order static->dynamic->skeletal.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
	EItemMeshType PreferredType;

	// If this component is allowed to exist without any mesh then enable this.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
	bool AllowNullMeshes = false;

	// If the mesh data does not have the preferred type, this stores the actual type used.
	UPROPERTY(BlueprintReadOnly, Category = "State")
	EItemMeshType ActualType;

	// Mesh data source.
	UPROPERTY(BlueprintReadOnly, Category = "State")
	FFaerieItemMesh MeshData;

	// Component generated at runtime to display the appropriate mesh from MeshData.
	UPROPERTY(BlueprintReadOnly, Category = "State")
	TObjectPtr<UMeshComponent> MeshComponent;
};