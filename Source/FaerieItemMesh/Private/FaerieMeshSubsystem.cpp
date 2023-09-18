// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "FaerieMeshSubsystem.h"

#include "FaerieItem.h"
#include "FaerieItemDataProxy.h"
#include "FaerieMeshStructs.h"
#include "FaerieMeshToken.h"
#include "SkeletalMergingLibrary.h"

#include "Engine/StaticMeshSocket.h"
#include "Engine/SkeletalMeshSocket.h"

#include "UDynamicMesh.h" // For creating static meshes at runtime

#include "GeometryScript/MeshAssetFunctions.h"
#include "GeometryScript/MeshBasicEditFunctions.h"
#include "GeometryScript/MeshMaterialFunctions.h"

void UFaerieMeshSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	FallbackPurpose = Faerie::ItemMesh::Tags::MeshPurpose_Default;
}

FFaerieItemMesh UFaerieMeshSubsystem::GetDynamicStaticMeshForData(const FFaerieDynamicStaticMesh& MeshData)
{
	if (MeshData.Fragments.IsEmpty())
	{
		return FFaerieItemMesh();
	}

	// The final mesh we will return.
	UDynamicMesh* OutMesh = NewObject<UDynamicMesh>();
	TArray<FFaerieItemMaterial> Materials;

	TMap<FName, UStaticMeshSocket*> Sockets;

	for (const FFaerieDynamicStaticMeshFragment& Fragment : MeshData.Fragments)
	{
		if (!Fragment.StaticMesh.IsValid())
		{
			UE_LOG(LogTemp, Error, TEXT("Invalid Static Mesh detected while building dynamic mesh!"))
			continue;
		}

		// Copy mesh data

		UDynamicMesh* AppendMesh = NewObject<UDynamicMesh>();

		const FGeometryScriptCopyMeshFromAssetOptions AssetOptions;
		const FGeometryScriptMeshReadLOD RequestedLOD;
		EGeometryScriptOutcomePins Outcome;
		UGeometryScriptLibrary_StaticMeshFunctions::CopyMeshFromStaticMesh(Fragment.StaticMesh.LoadSynchronous(), AppendMesh, AssetOptions, RequestedLOD, Outcome);

		for (auto&& Socket : Fragment.StaticMesh->Sockets)
		{
			Sockets.Add(Socket->SocketName, Socket);
		}

		// Figure out mesh transform

		FTransform AppendTransform = Fragment.Attachment.Offset;

		if (!Fragment.Attachment.Socket.IsNone())
		{
			if (auto&& Socket = Sockets.Find(Fragment.Attachment.Socket);
				Socket && IsValid(*Socket))
			{
				AppendTransform *= FTransform((*Socket)->RelativeRotation, (*Socket)->RelativeLocation, (*Socket)->RelativeScale);
			}
		}

		// Align material IDs

		const int32 NumStaticMaterials = Fragment.StaticMesh->GetStaticMaterials().Num();
		const int32 NumDynamicMaterials = Fragment.Materials.Num();
		const int32 MaterialOverrideNum = FMath::Min(NumStaticMaterials, NumDynamicMaterials);

		for (int32 MatOverrideIndex = 0; MatOverrideIndex < MaterialOverrideNum; ++MatOverrideIndex)
		{
			if (const int32 ExistingIndex = Materials.IndexOfByPredicate(
				[&](const FFaerieItemMaterial& IndexedMat)
				{
					return IndexedMat.Material == Fragment.StaticMesh->GetMaterial(MatOverrideIndex);
				});
				ExistingIndex != INDEX_NONE)
			{
				UGeometryScriptLibrary_MeshMaterialFunctions::RemapMaterialIDs(AppendMesh, MatOverrideIndex, ExistingIndex);
			}
			else
			{
				const int32 NewIndex = Materials.Add(Fragment.Materials[MatOverrideIndex]);
				UGeometryScriptLibrary_MeshMaterialFunctions::RemapMaterialIDs(AppendMesh, MatOverrideIndex, NewIndex);
			}
		}

		// Commit new mesh
		UGeometryScriptLibrary_MeshBasicEditFunctions::AppendMesh(OutMesh, AppendMesh, AppendTransform);

		// Trash temp dynamic mesh.
		AppendMesh->MarkAsGarbage();
	}

	return FFaerieItemMesh::MakeDynamic(OutMesh, Materials);
}

FFaerieItemMesh UFaerieMeshSubsystem::GetDynamicSkeletalMeshForData(const FFaerieDynamicSkeletalMesh& MeshData) const
{
	FSkeletonAndAnimClass OutSkeletonAndAnimClass;
	TArray<FFaerieItemMaterial> Materials;

	FSkeletalMeshMergeParams Params;
	OutSkeletonAndAnimClass.Mesh = USkeletalMergingLibrary::MergeMeshes(Params);

	return FFaerieItemMesh::MakeSkeletal(OutSkeletonAndAnimClass, Materials);
}

bool UFaerieMeshSubsystem::LoadMeshFromTokenSynchronous(const UFaerieMeshTokenBase* Token, const FGameplayTag Purpose,
	FFaerieItemMesh& Mesh)
{
	// This is a stupid fix for an issue with blueprints, where impure nodes will cache their output across multiple executions.
	// The result without this line, is that a mesh value put into Mesh in one call will persist for successive calls.
	Mesh = FFaerieItemMesh();

	if (!IsValid(Token))
	{
		UE_LOG(LogTemp, Warning, __FUNCTION__ TEXT(": No MeshToken on entry"))
		return false;
	}

	const FFaerieCachedMeshKey Key = {Token, Purpose};

	// If we have already generated this mesh, just return that one.
	if (auto&& CachedMesh = GeneratedMeshes.Find(Key))
	{
		Mesh = *CachedMesh;
		return true;
	}

	FGameplayTagContainer PurposeHierarchy;
	if (Purpose != Faerie::ItemMesh::Tags::MeshPurpose_Default)
	{
		PurposeHierarchy.AddTagFast(Purpose);
	}
	if (FallbackPurpose.IsValid() && FallbackPurpose != Faerie::ItemMesh::Tags::MeshPurpose_Default)
	{
		PurposeHierarchy.AddTagFast(FallbackPurpose);
	}
	PurposeHierarchy.AddTagFast(Faerie::ItemMesh::Tags::MeshPurpose_Default);

	// Check for the presence of a custom dynamic mesh to build.

	if (auto&& DynamicMeshToken = Cast<UFaerieMeshToken_Dynamic>(Token))
	{
		for (auto&& SkeletalMesh : DynamicMeshToken->DynamicMeshContainer.SkeletalMeshes)
		{
			if (SkeletalMesh.Purpose.HasAnyExact(PurposeHierarchy))
			{
				Mesh = GetDynamicSkeletalMeshForData(SkeletalMesh);

				if (Mesh.IsSkeletal())
				{
					GeneratedMeshes.Add(Key, Mesh);
					return true;
				}
			}
		}

		for (auto&& StaticMesh : DynamicMeshToken->DynamicMeshContainer.StaticMeshes)
		{
			if (StaticMesh.Purpose.HasAnyExact(PurposeHierarchy))
			{
				Mesh = GetDynamicStaticMeshForData(StaticMesh);

				if (Mesh.IsDynamic())
				{
					GeneratedMeshes.Add(Key, Mesh);
					return true;
				}
			}
		}
	}


	// Otherwise, scan and load pre-defined mesh data.

	if (FFaerieSkeletalMeshData SkelMeshData;
		Token->GetSkeletalItemMesh(PurposeHierarchy, SkelMeshData))
	{
		Mesh = FFaerieItemMesh::MakeSkeletal(SkelMeshData.SkeletonAndAnimClass.LoadSynchronous(), SkelMeshData.Materials);
		GeneratedMeshes.Add(Key, Mesh);
		return true;
	}

	if (FFaerieStaticMeshData StaticMeshData;
		Token->GetStaticItemMesh(PurposeHierarchy, StaticMeshData))
	{
		Mesh = FFaerieItemMesh::MakeStatic(StaticMeshData.StaticMesh.LoadSynchronous(), StaticMeshData.Materials);
		GeneratedMeshes.Add(Key, Mesh);
		return true;
	}

	UE_LOG(LogTemp, Error, __FUNCTION__ TEXT(": Asset does not contain a mesh suitable for the purpose."))
	return false;
}

bool UFaerieMeshSubsystem::LoadMeshFromProxySynchronous(const FFaerieItemProxy Proxy, const FGameplayTag Purpose,
														FFaerieItemMesh& Mesh)
{
	// This is a stupid fix for an issue with blueprints, where impure nodes will cache their output across multiple executions.
	// The result without this line, is that a mesh value put into Mesh in one call will persist for successive calls.
	Mesh = FFaerieItemMesh();

	if (!ensure(Proxy.IsValid()))
	{
		UE_LOG(LogTemp, Warning, __FUNCTION__ TEXT(": Invalid proxy!"))
		return false;
	}

	if (!IsValid(Proxy->GetItemObject()))
	{
		UE_LOG(LogTemp, Error, __FUNCTION__ TEXT(": Invalid item object!"))
		return false;
	}

	if (auto&& MeshToken = Proxy->GetItemObject()->GetToken<UFaerieMeshTokenBase>())
	{
		return LoadMeshFromTokenSynchronous(MeshToken, Purpose, Mesh);
	}
	return false;
}