// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "Animation/AnimInstance.h"
#include "AnimUtilityStructs.generated.h"

USTRUCT(BlueprintType)
struct FAERIEITEMMESH_API FSocketAttachment
{
	GENERATED_BODY()

	/** Attach to this bone */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Attachment Data")
	FName Socket;

	/** Attach with this offset */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Attachment Data")
	FTransform Offset;
};

/*
 * A skeletal mesh and paired animation blueprint class.
*/
USTRUCT(BlueprintType)
struct FAERIEITEMMESH_API FSkeletonAndAnimClass
{
	// Empty constructor
	FSkeletonAndAnimClass() {}

	// Full constructor
	FSkeletonAndAnimClass(USkeletalMesh* Mesh, const TSubclassOf<UAnimInstance> AnimClass)
		: Mesh(Mesh),
		AnimClass(AnimClass) {}

	GENERATED_BODY()

	UPROPERTY(NoClear, BlueprintReadWrite, EditAnywhere, Category = "Skeletal Mesh and Anim Class")
	TObjectPtr<USkeletalMesh> Mesh = nullptr;

	UPROPERTY(NoClear, BlueprintReadWrite, EditAnywhere, Category = "Skeletal Mesh and Anim Class")
	TSubclassOf<UAnimInstance> AnimClass = nullptr;

	friend bool operator==(const FSkeletonAndAnimClass& Lhs, const FSkeletonAndAnimClass& Rhs)
	{
		return Lhs.Mesh == Rhs.Mesh
			&& Lhs.AnimClass == Rhs.AnimClass;
	}

	friend bool operator!=(const FSkeletonAndAnimClass& Lhs, const FSkeletonAndAnimClass& Rhs)
	{
		return !(Lhs == Rhs);
	}
};

FORCEINLINE uint32 GetTypeHash(const FSkeletonAndAnimClass& Thing)
{
	return FCrc::MemCrc32(&Thing, sizeof(FSkeletonAndAnimClass));
}