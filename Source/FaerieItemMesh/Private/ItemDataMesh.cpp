// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "ItemDataMesh.h"
#include "FaerieMeshStructs.h"
#include "FaerieMeshStructsLibrary.h"
#include "Components/DynamicMeshComponent.h"
#include "GeometryScript/MeshQueryFunctions.h"

UFaerieItemMeshComponent::UFaerieItemMeshComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UFaerieItemMeshComponent::RebuildMesh()
{
	EItemMeshType NewMeshType = EItemMeshType::None;

	// Select new type by preferred first
	switch (PreferredType)
	{
	case EItemMeshType::None: break;
	case EItemMeshType::Static:
		if (IsValid(MeshData.StaticMesh))
		{
			NewMeshType = EItemMeshType::Static;
		}
		break;
	case EItemMeshType::Dynamic:
		if (IsValid(MeshData.DynamicStaticMesh))
		{
			NewMeshType = EItemMeshType::Dynamic;
		}
		break;
	case EItemMeshType::Skeletal:
		if (IsValid(MeshData.SkeletonAndAnimClass.Mesh)
		 && IsValid(MeshData.SkeletonAndAnimClass.AnimClass))
		{
			NewMeshType = EItemMeshType::Skeletal;
		}
		break;
	default: checkNoEntry();
	}

	// If the new type is not valid yet, select by what is actually present instead.
	if (NewMeshType == EItemMeshType::None)
	{
		if (IsValid(MeshData.StaticMesh))
		{
			NewMeshType = EItemMeshType::Static;
		}
		else if (IsValid(MeshData.DynamicStaticMesh))
		{
			NewMeshType = EItemMeshType::Dynamic;
		}
		else if (IsValid(MeshData.SkeletonAndAnimClass.Mesh)
			  && IsValid(MeshData.SkeletonAndAnimClass.AnimClass))
		{
			NewMeshType = EItemMeshType::Skeletal;
		}
	}

	// If we are switching types, then destroy the old component if it exists.
	if (NewMeshType != ActualType)
	{
		if (IsValid(MeshComponent))
		{
			MeshComponent->DestroyComponent();
			MeshComponent = nullptr;
		}

		ActualType = NewMeshType;
	}

	// Warn if we have switched to None illegally.
	if (NewMeshType == EItemMeshType::None && !AllowNullMeshes)
	{
		//@todo log category
		UE_LOG(LogTemp, Error, TEXT("No valid mesh in Mesh Data. RebuildMesh cancelled."))
		return;
	}

	// Create the appropriate new mesh component for the type.
	if (!IsValid(MeshComponent))
	{
		switch (ActualType)
		{
		case EItemMeshType::None: break;
		case EItemMeshType::Static:
			MeshComponent = NewObject<UStaticMeshComponent>(GetOwner());
			break;
		case EItemMeshType::Dynamic:
			MeshComponent = NewObject<UDynamicMeshComponent>(GetOwner());
			break;
		case EItemMeshType::Skeletal:
			MeshComponent = NewObject<USkeletalMeshComponent>(GetOwner());
			break;
		default: checkNoEntry();
		}

		MeshComponent->RegisterComponent();

		// *This* component controls the transform, so hard snap the mesh component to ourself.
		MeshComponent->AttachToComponent(this, FAttachmentTransformRules::SnapToTargetIncludingScale);
	}

	// Load the mesh and materials to the mesh component.
	switch (ActualType)
	{
	case EItemMeshType::None: break;
	case EItemMeshType::Static:
		if (UStaticMeshComponent* StaticMesh = Cast<UStaticMeshComponent>(MeshComponent))
		{
			StaticMesh->SetStaticMesh(MeshData.StaticMesh);
			for (int32 MaterialIndex = 0; MaterialIndex < MeshData.Materials.Num(); ++MaterialIndex)
			{
				StaticMesh->SetMaterial(MaterialIndex, MeshData.Materials[MaterialIndex].Material);
			}
		}
		break;
	case EItemMeshType::Dynamic:
		if (UDynamicMeshComponent* DynamicMesh = Cast<UDynamicMeshComponent>(MeshComponent))
		{
			DynamicMesh->SetDynamicMesh(MeshData.DynamicStaticMesh);
			DynamicMesh->ConfigureMaterialSet(UFaerieMeshStructsLibrary::FaerieItemMaterialsToObjectArray(MeshData.Materials));
		}
		break;
	case EItemMeshType::Skeletal:
		if (USkeletalMeshComponent* SkeletalMesh = Cast<USkeletalMeshComponent>(MeshComponent))
		{
			SkeletalMesh->SetSkeletalMesh(MeshData.SkeletonAndAnimClass.Mesh);
			SkeletalMesh->SetAnimInstanceClass(MeshData.SkeletonAndAnimClass.AnimClass);
			for (int32 MaterialIndex = 0; MaterialIndex < MeshData.Materials.Num(); ++MaterialIndex)
			{
				SkeletalMesh->SetMaterial(MaterialIndex, MeshData.Materials[MaterialIndex].Material);
			}
		}
		break;
	default: checkNoEntry();
	}
}

void UFaerieItemMeshComponent::SetItemMesh(const FFaerieItemMesh& InMeshData)
{
	if (MeshData != InMeshData)
	{
		MeshData = InMeshData;
		RebuildMesh();
	}
}

void UFaerieItemMeshComponent::ClearItemMesh()
{
	ActualType = EItemMeshType::None;
	MeshData = FFaerieItemMesh();

	if (IsValid(MeshComponent))
	{
		MeshComponent->DestroyComponent();
		MeshComponent = nullptr;
	}
}

void UFaerieItemMeshComponent::SetPreferredMeshType(const EItemMeshType MeshType)
{
	if (PreferredType != MeshType)
	{
		PreferredType = MeshType;
		RebuildMesh();
	}
}

FBoxSphereBounds UFaerieItemMeshComponent::GetBounds() const
{
	switch (ActualType) {
	case EItemMeshType::Static: return MeshData.StaticMesh->GetBounds();
	case EItemMeshType::Dynamic: return UGeometryScriptLibrary_MeshQueryFunctions::GetMeshBoundingBox(MeshData.DynamicStaticMesh);
	case EItemMeshType::Skeletal: return MeshData.SkeletonAndAnimClass.Mesh->GetBounds();
	default: return FBoxSphereBounds();
	}
}