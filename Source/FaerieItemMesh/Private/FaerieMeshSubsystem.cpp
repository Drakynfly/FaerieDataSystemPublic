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


UDynamicMeshPool* UFaerieMeshSubsystem::GetMeshPool()
{
	if (!MeshPool) MeshPool = NewObject<UDynamicMeshPool>();
	return MeshPool;
}

UDynamicMesh* UFaerieMeshSubsystem::GetDynamicStaticMeshForData(const FFaerieDynamicStaticMesh& MeshData,
                                                                TArray<FFaerieItemMaterial>& OutMaterialSet)
{
	if (MeshData.Fragments.IsEmpty())
	{
		return nullptr;
	}

	// The final mesh we will return.
	UDynamicMesh* OutMesh = GetMeshPool()->RequestMesh();

	TMap<FName, UStaticMeshSocket*> Sockets;

	for (const FFaerieDynamicStaticMeshFragment& Fragment : MeshData.Fragments)
	{
		if (!IsValid(Fragment.StaticMesh))
		{
			UE_LOG(LogTemp, Error, TEXT("Invalid Static Mesh detected while building dynamic mesh!"))
			continue;
		}

		// Copy mesh data

		UDynamicMesh* AppendMesh = GetMeshPool()->RequestMesh();

		const FGeometryScriptCopyMeshFromAssetOptions AssetOptions;
		const FGeometryScriptMeshReadLOD RequestedLOD;
		EGeometryScriptOutcomePins Outcome;
		UGeometryScriptLibrary_StaticMeshFunctions::CopyMeshFromStaticMesh(Fragment.StaticMesh, AppendMesh, AssetOptions, RequestedLOD, Outcome);

		for (auto&& Socket : Fragment.StaticMesh->Sockets)
		{
			Sockets.Add(Socket->SocketName, Socket);
		}

		// Figure out mesh transform

		FTransform AppendTransform = Fragment.Attachment.Offset;

		if (!Fragment.Attachment.Socket.IsNone())
		{
			auto&& Socket = Sockets.Find(Fragment.Attachment.Socket);

			if (Socket && IsValid(*Socket))
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
			const int32 ExistingIndex = OutMaterialSet.IndexOfByPredicate([&](const FFaerieItemMaterial& IndexedMat)
			{
				return IndexedMat.Material == Fragment.StaticMesh->GetMaterial(MatOverrideIndex);
			});

			if (ExistingIndex != INDEX_NONE)
			{
				UGeometryScriptLibrary_MeshMaterialFunctions::RemapMaterialIDs(AppendMesh, MatOverrideIndex, ExistingIndex);
			}
			else
			{
				const int32 NewIndex = OutMaterialSet.Add(Fragment.Materials[MatOverrideIndex]);
				UGeometryScriptLibrary_MeshMaterialFunctions::RemapMaterialIDs(AppendMesh, MatOverrideIndex, NewIndex);
			}
		}

		// Commit new mesh
		UGeometryScriptLibrary_MeshBasicEditFunctions::AppendMesh(OutMesh, AppendMesh, AppendTransform);

		// Discard temp dynamic mesh.
		GetMeshPool()->ReturnMesh(AppendMesh);
	}

	return OutMesh;
}

FSkeletonAndAnimClass UFaerieMeshSubsystem::GetDynamicSkeletalMeshForData(const FFaerieDynamicSkeletalMesh& MeshData,
                                                                   TArray<FFaerieItemMaterial>& OutMaterialSet)
{
	FSkeletonAndAnimClass OutSkeletonAndAnimClass;

	FSkeletalMeshMergeParams Params;

	OutSkeletonAndAnimClass.Mesh = USkeletalMergingLibrary::MergeMeshes(Params);

	return OutSkeletonAndAnimClass;
}

bool UFaerieMeshSubsystem::LoadMeshFromTokenSynchronous(UFaerieMeshTokenBase* Token, const FGameplayTag Purpose,
	FFaerieItemMesh& Mesh)
{
	// This is a stupid fix for an issue with blueprints, where impure nodes will cache their output across multiple executions.
	// The result without this line, is that a static mesh fed into Mesh in one call will persist for successive calls.
	Mesh = FFaerieItemMesh();

	if (!IsValid(Token))
	{
		UE_LOG(LogTemp, Warning, __FUNCTION__ TEXT(": No MeshToken on entry"))
		return false;
	}

	const FFaerieCachedMeshKey Key = {Token, Purpose};

	if (auto&& CachedMesh = GeneratedMeshes.Find(Key))
	{
		Mesh = *CachedMesh;
		return true;
	}

	FGameplayTagContainer PurposeHierarchy;
	if (Purpose != FMeshPurposeTags::Get().MP_Default)
	{
		PurposeHierarchy.AddTagFast(Purpose);
	}
	if (FallbackPurpose.IsValid() && FallbackPurpose != FMeshPurposeTags::Get().MP_Default)
	{
		PurposeHierarchy.AddTagFast(FallbackPurpose);
	}
	PurposeHierarchy.AddTagFast(FMeshPurposeTags::Get().MP_Default);

	// Check for the presence of a custom dynamic mesh to build.

	if (auto&& DynamicMeshToken = Cast<UFaerieMeshToken_Dynamic>(Token))
	{
		for (auto&& SkeletalMesh : DynamicMeshToken->DynamicMeshContainer.SkeletalMeshes)
		{
			if (SkeletalMesh.Purpose.HasAnyExact(PurposeHierarchy))
			{
				Mesh.SkeletonAndAnimClass = GetDynamicSkeletalMeshForData(SkeletalMesh, Mesh.Materials);

				if (IsValid(Mesh.SkeletonAndAnimClass.Mesh) &&
					IsValid(Mesh.SkeletonAndAnimClass.AnimClass))
				{
					return true;
				}
			}
		}

		for (auto&& StaticMesh : DynamicMeshToken->DynamicMeshContainer.StaticMeshes)
		{
			if (StaticMesh.Purpose.HasAnyExact(PurposeHierarchy))
			{
				Mesh.DynamicStaticMesh = GetDynamicStaticMeshForData(StaticMesh, Mesh.Materials);

				if (IsValid(Mesh.DynamicStaticMesh))
				{
					return true;
				}
			}
		}
	}


	// Otherwise, scan and load pre-defined mesh data.

	FFaerieSkeletalMeshData SkelMeshData;
	if (Token->GetSkeletalItemMesh(PurposeHierarchy, SkelMeshData))
	{
		Mesh.SkeletonAndAnimClass = SkelMeshData.SkeletonAndAnimClass;
		Mesh.Materials = SkelMeshData.Materials;
		return true;
	}

	FFaerieStaticMeshData StaticMeshData;
	if (Token->GetStaticItemMesh(PurposeHierarchy, StaticMeshData))
	{
		Mesh.StaticMesh = StaticMeshData.StaticMesh;
		Mesh.Materials = StaticMeshData.Materials;
		return true;
	}

	GeneratedMeshes.Add(Key, Mesh);

	UE_LOG(LogTemp, Error, __FUNCTION__ TEXT(": Asset does not contain a mesh suitable for the purpose."))
	return false;
}

bool UFaerieMeshSubsystem::LoadMeshFromReaderSynchronous(UFaerieItemDataProxyBase* Proxy, const FGameplayTag Purpose, FFaerieItemMesh& Mesh)
{
	// This is a stupid fix for an issue with blueprints, where impure nodes will cache their output across multiple executions.
	// The result without this line, is that a static mesh fed into Mesh in one call will persist for successive calls.
	Mesh = FFaerieItemMesh();

	if (!ensure(IsValid(Proxy)))
	{
		UE_LOG(LogTemp, Warning, __FUNCTION__ TEXT(": Invalid proxy!"))
		return false;
	}

	if (!IsValid(Proxy->GetItemObject()))
	{
		UE_LOG(LogTemp, Error, __FUNCTION__ TEXT(": Invalid item object!"))
		return false;
	}

	auto&& MeshToken = Proxy->GetItemObject()->GetToken<UFaerieMeshTokenBase>();

	return LoadMeshFromTokenSynchronous(MeshToken, Purpose, Mesh);
}
