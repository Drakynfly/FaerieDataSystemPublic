// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "FaerieInfoObject.h"

#include "FaerieItemSource.generated.h"

class UFaerieItem;

UCLASS(Const)
class FAERIEITEMDATA_API UItemInstancingContext : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TObjectPtr<UObject> Outer = nullptr;
};

// This class does not need to be modified.
UINTERFACE(BlueprintType, meta = (CannotImplementInterfaceInBlueprint))
class FAERIEITEMDATA_API UFaerieItemSource : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface added to classes that can author a faerie item instance
 */
class FAERIEITEMDATA_API IFaerieItemSource
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	// Can this source create mutable items.
	virtual bool CanBeMutable() const { return false; }

	// Allows sources to give info about generation results
	UFUNCTION(BlueprintCallable, Category = "Faerie|ItemSource")
	virtual FFaerieAssetInfo GetSourceInfo() const { return FFaerieAssetInfo(); }

	// Creates an item with the given outer.
	virtual UFaerieItem* CreateItemInstance(UObject* Outer = GetTransientPackage()) const;

	// Creates an item that can utilize contextual data given to it by the requester of the item.
	virtual UFaerieItem* CreateItemInstance(const UItemInstancingContext* Context) const;
};

/**
 * A wrapper struct that can container a pointer to any object that implements IFaerieItemSource
 */
USTRUCT(BlueprintType)
struct FAERIEITEMDATA_API FFaerieItemSourceObject
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowedClasses = "/Script/FaerieItemData.FaerieItemSource"))
	TSoftObjectPtr<UObject> Object;

	friend bool operator==(const FFaerieItemSourceObject& Lhs, const FFaerieItemSourceObject& Rhs)
	{
		return Lhs.Object == Rhs.Object;
	}

	friend bool operator!=(const FFaerieItemSourceObject& Lhs, const FFaerieItemSourceObject& Rhs)
	{
		return !(Lhs == Rhs);
	}
};

FORCEINLINE uint32 GetTypeHash(const FFaerieItemSourceObject& Value)
{
	return GetTypeHash(Value.Object);
}