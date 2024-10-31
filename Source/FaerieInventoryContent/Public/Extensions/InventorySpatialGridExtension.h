// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "ItemContainerExtensionBase.h"
#include "SpatialStructs.h"
#include "InventorySpatialGridExtension.generated.h"

class UInventorySpatialGridExtension;
class UFaerieShapeToken;

USTRUCT()
struct FSpatialEntryKey
{
	GENERATED_BODY()

	UPROPERTY(VisibleInstanceOnly, Category = "SpatialEntryKey")
	FIntPoint Key;

	friend bool operator<(const FSpatialEntryKey& A, const FSpatialEntryKey& B)
	{
		return A.Key.X < B.Key.X || (A.Key.X == B.Key.X && A.Key.Y < B.Key.Y);
	}

	friend bool operator==(const FSpatialEntryKey& A, const FSpatialEntryKey& B)
	{
		return A.Key.X == B.Key.X && A.Key.Y == B.Key.Y;
	}
};

struct FSpatialContent;

USTRUCT(BlueprintType)
struct FSpatialKeyedEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()

	FSpatialKeyedEntry() = default;

	FSpatialKeyedEntry(const FSpatialEntryKey Key, const FEntryKey Value)
		: Key(Key), Value(Value)
	{
	}

	UPROPERTY(VisibleInstanceOnly, Category = "SpatialKeyedEntry")
	FSpatialEntryKey Key;

	UPROPERTY(VisibleInstanceOnly, Category = "SpatialKeyedEntry")
	FEntryKey Value;

	void PreReplicatedRemove(const FSpatialContent& InArraySerializer);
	void PostReplicatedAdd(FSpatialContent& InArraySerializer);
	void PostReplicatedChange(const FSpatialContent& InArraySerializer);
};

USTRUCT(BlueprintType)
struct FSpatialContent : public FFastArraySerializer,
                         public TBinarySearchOptimizedArray<FSpatialContent, FSpatialKeyedEntry>
{
	GENERATED_BODY()

	friend TBinarySearchOptimizedArray;
	friend UInventorySpatialGridExtension;

private:
	UPROPERTY(VisibleAnywhere, Category = "SpatialContent")
	TArray<FSpatialKeyedEntry> Items;

	TArray<FSpatialKeyedEntry>& GetArray() { return Items; }

	/** Owning storage to send Fast Array callbacks to */
	UPROPERTY()
	TWeakObjectPtr<UInventorySpatialGridExtension> ChangeListener;

public:
	TConstArrayView<FSpatialKeyedEntry> GetEntries() const { return Items; };

	void PreEntryReplicatedRemove(const FSpatialKeyedEntry& Entry) const;
	void PostEntryReplicatedAdd(const FSpatialKeyedEntry& Entry);
	void PostEntryReplicatedChange(const FSpatialKeyedEntry& Entry) const;

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams)
	{
		return FastArrayDeltaSerialize<FSpatialKeyedEntry, FSpatialContent>(Items, DeltaParams, *this);
	}

	void Insert(FSpatialEntryKey Key, FEntryKey Value);

	void Remove(FSpatialEntryKey Key);
};

template <>
struct TStructOpsTypeTraits<FSpatialContent> : public TStructOpsTypeTraitsBase2<FSpatialContent>
{
	enum
	{
		WithNetDeltaSerializer = true,
	};
};

using FSpatialEntryChangedNative = TMulticastDelegate<void(FEntryKey)>;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSpatialEntryChanged, FEntryKey, EntryKey);

using FGridSizeChangedNative = TMulticastDelegate<void(FIntPoint)>;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGridSizeChanged, FIntPoint, newGridSize);

/**
 *
 */
UCLASS()
class FAERIEINVENTORYCONTENT_API UInventorySpatialGridExtension : public UItemContainerExtensionBase
{
	GENERATED_BODY()

	friend FSpatialContent;

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitProperties() override;

protected:
	//~ UItemContainerExtensionBase
	virtual EEventExtensionResponse AllowsAddition(const UFaerieItemContainerBase* Container,
	                                               FFaerieItemStackView Stack) override;
	virtual void PostAddition(const UFaerieItemContainerBase* Container,
	                          const Faerie::Inventory::FEventLog& Event) override;
	virtual void PostRemoval(const UFaerieItemContainerBase* Container,
	                         const Faerie::Inventory::FEventLog& Event) override;
	//~ UItemContainerExtensionBase

	void PreEntryReplicatedRemove(const FSpatialKeyedEntry& Entry);
	void PostEntryReplicatedAdd(const FSpatialKeyedEntry& Entry);
	void PostEntryReplicatedChange(const FSpatialKeyedEntry& Entry);

public:
	bool CanAddItemToGrid(const UFaerieShapeToken* ShapeToken, const FIntPoint& Position) const;
	bool CanAddItemToGrid(const UFaerieShapeToken* ShapeToken) const;

	bool MoveItem(const FEntryKey& Key, const FIntPoint& SourcePoint, const FIntPoint& TargetPoint);
	bool RotateItem(const FEntryKey& Key, const FIntPoint& PivotPoint);

	bool AddItemToGrid(const FEntryKey& Key, const UFaerieShapeToken* ShapeToken);
	void RemoveItemFromGrid(const FEntryKey& Key);

	UFUNCTION(BlueprintCallable, Category = "Grid")
	FFaerieGridShape GetEntryPositions(const FEntryKey& Key) const;
	bool FitsInGrid(const FFaerieGridShape& Shape, const FIntPoint& Position, const FSpatialContent& Occupied,
	                const bool bCheckingRotation = false,
	                const FEntryKey& ExcludeKey = FEntryKey()) const;
	TOptional<FIntPoint> GetFirstEmptyLocation(const FFaerieGridShape& InShape, const FSpatialContent& Occupied) const;

	FSpatialEntryChangedNative& GetOnSpatialEntryChanged() { return SpatialEntryChangedDelegateNative; }
	FSpatialContent& GetContent() { return OccupiedSlots; }

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Grid")
	void SetGridSize(FIntPoint NewGridSize);

protected:
	UPROPERTY(BlueprintAssignable, Category = "Delegates")
	FSpatialEntryChanged SpatialEntryChangedDelegate;

	UPROPERTY(BlueprintAssignable, Category = "Delegates")
	FGridSizeChanged GridSizeChangedDelegate;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, ReplicatedUsing=OnRep_GridSize, Category = "Config")
	FIntPoint GridSize = FIntPoint(10, 40);

	UPROPERTY(EditAnywhere, Replicated, Category = "Data")
	FSpatialContent OccupiedSlots;

private:
	FSpatialEntryChangedNative SpatialEntryChangedDelegateNative;
	FGridSizeChangedNative GridSizeChangedDelegateNative;
	UFUNCTION()
	void OnRep_GridSize();
};
