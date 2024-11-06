// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "ItemContainerExtensionBase.h"
#include "SpatialStructs.h"
#include "InventorySpatialGridExtension.generated.h"

class UInventorySpatialGridExtension;
class UFaerieShapeToken;

UENUM(BlueprintType)
enum class ESpatialItemRotation : uint8
{
	None = 0,
	Ninety = 1,
	One_Eighty = 2,
	Two_Seventy = 3
};

FORCEINLINE ESpatialItemRotation GetNextRotation(const ESpatialItemRotation CurrentRotation)
{
	switch (CurrentRotation)
	{
	case ESpatialItemRotation::None:
		return ESpatialItemRotation::Ninety;
	case ESpatialItemRotation::Ninety:
		return ESpatialItemRotation::One_Eighty;
	case ESpatialItemRotation::One_Eighty:
		return ESpatialItemRotation::Two_Seventy;
	case ESpatialItemRotation::Two_Seventy:
		return ESpatialItemRotation::None;
	default:
		return ESpatialItemRotation::None;
	}
}

USTRUCT(BlueprintType)
struct FSpatialItemPlacement
{
	GENERATED_BODY()

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "SpatialItemPlacement")
	FIntPoint Origin = FIntPoint::ZeroValue;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "SpatialItemPlacement")
	FIntPoint PivotPoint = FIntPoint::ZeroValue;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "SpatialItemPlacement")
	FFaerieGridShape ItemShape;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "SpatialItemPlacement")
	ESpatialItemRotation Rotation = ESpatialItemRotation::None;

	friend bool operator==(const FSpatialItemPlacement& A, const FSpatialItemPlacement& B)
	{
		return A.Origin == B.Origin && A.Rotation == B.Rotation;
	}

	friend bool operator<(const FSpatialItemPlacement& A, const FSpatialItemPlacement& B)
	{
		return A.Origin.X < B.Origin.X || (A.Origin.X == B.Origin.X && A.Origin.Y < B.Origin.Y);
	}
};

struct FSpatialContent;

USTRUCT(BlueprintType)
struct FSpatialKeyedEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()

	FSpatialKeyedEntry() = default;

	FSpatialKeyedEntry(const FInventoryKey Key, const FSpatialItemPlacement& Value)
		: Key(Key), Value(Value)
	{
	}

	UPROPERTY(VisibleInstanceOnly, Category = "SpatialKeyedEntry")
	FInventoryKey Key;

	UPROPERTY(VisibleInstanceOnly, Category = "SpatialKeyedEntry")
	FSpatialItemPlacement Value;

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
	bool EditItem(FInventoryKey Key, const TFunctionRef<void(FSpatialItemPlacement&)>& Func);

	void PreEntryReplicatedRemove(const FSpatialKeyedEntry& Entry) const;
	void PostEntryReplicatedAdd(const FSpatialKeyedEntry& Entry);
	void PostEntryReplicatedChange(const FSpatialKeyedEntry& Entry) const;

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams)
	{
		return FastArrayDeltaSerialize<FSpatialKeyedEntry, FSpatialContent>(Items, DeltaParams, *this);
	}

	void Insert(FInventoryKey Key, const FSpatialItemPlacement& Value);

	void Remove(FInventoryKey Key);

	// Only const iteration is allowed.
	using TRangedForConstIterator = TArray<FSpatialKeyedEntry>::RangedForConstIteratorType;
	FORCEINLINE TRangedForConstIterator begin() const { return TRangedForConstIterator(Items.begin()); }
	FORCEINLINE TRangedForConstIterator end() const { return TRangedForConstIterator(Items.end()); }
};

template <>
struct TStructOpsTypeTraits<FSpatialContent> : public TStructOpsTypeTraitsBase2<FSpatialContent>
{
	enum
	{
		WithNetDeltaSerializer = true,
	};
};

UENUM(BlueprintType)
enum ESpatialEventType : uint8
{
	ItemAdded,
	ItemChanged,
	ItemRemoved
};

using FSpatialEntryChangedNative = TMulticastDelegate<void(const FInventoryKey&, ESpatialEventType)>;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSpatialEntryChanged, FInventoryKey, EntryKey, ESpatialEventType,
                                             EventType);

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
	virtual void InitializeExtension(const UFaerieItemContainerBase* Container) override;
	virtual void DeinitializeExtension(const UFaerieItemContainerBase* Container) override;
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

	UFUNCTION(/* Replication */)
	virtual void OnRep_GridSize();

private:
	bool AddItemToGrid(const FInventoryKey& Key, const UFaerieShapeToken* ShapeToken);
	void RemoveItem(const FInventoryKey& Key);
	void RemoveItemsForEntry(const FEntryKey& Key);

public:
	bool CanAddItemToGrid(const UFaerieShapeToken* ShapeToken, const FIntPoint& Position) const;
	bool CanAddItemToGrid(const UFaerieShapeToken* ShapeToken) const;

	bool MoveItem(const FInventoryKey& Key, const FIntPoint& SourcePoint, const FIntPoint& TargetPoint);
	bool RotateItem(const FInventoryKey& Key);


	// @todo probably split into two functions. one with rotation check, one without. public API probably doesn't need to see the rotation check!
	bool FitsInGrid(const FFaerieGridShape& Shape, const FIntPoint& Position,
	                ESpatialItemRotation Rotation = ESpatialItemRotation::None,
	                TConstArrayView<FInventoryKey> ExcludedKeys = {}) const;

	TOptional<TTuple<FIntPoint, ESpatialItemRotation>> GetFirstEmptyLocation(const FFaerieGridShape& InShape) const;

	FSpatialEntryChangedNative& GetOnSpatialEntryChanged() { return SpatialEntryChangedDelegateNative; }
	FGridSizeChangedNative& GetOnGridSizeChanged() { return GridSizeChangedDelegateNative; }

	UFUNCTION(BlueprintCallable, Category = "Grid")
	FFaerieGridShape GetEntryShape(const FInventoryKey& Key) const;

	UFUNCTION(BlueprintCallable, Category = "Grid")
	FSpatialItemPlacement GetEntryPlacementData(const FInventoryKey& Key) const;

	// @todo Drakyn: move static
	UFUNCTION(BlueprintCallable, Category="Shape Manipulation")
	static FFaerieGridShape RotateShape(FFaerieGridShape InShape, const ESpatialItemRotation Rotation);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Grid")
	void SetGridSize(FIntPoint NewGridSize);

protected:
	FSpatialKeyedEntry* FindItemByKey(const FInventoryKey& Key);

	static bool ValidateSourcePoint(const FSpatialKeyedEntry* Entry, const FIntPoint& SourcePoint);

	// @todo Drakyn: look at these
	FSpatialKeyedEntry* FindOverlappingItem(const FFaerieGridShape& Shape, const FIntPoint& Offset,
	                                        const FInventoryKey& ExcludeKey);
	bool TrySwapItems(FSpatialKeyedEntry& MovingItem, FSpatialKeyedEntry& OverlappingItem, const FIntPoint& Offset);
	bool MoveSingleItem(FSpatialKeyedEntry& Item, const FIntPoint& Offset);
	void UpdateItemPosition(FSpatialKeyedEntry& Item, const FIntPoint& Offset);

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FSpatialEntryChanged SpatialEntryChangedDelegate;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FGridSizeChanged GridSizeChangedDelegate;

	// @todo should default to 10, 10
	UPROPERTY(EditAnywhere, BlueprintReadOnly, ReplicatedUsing = "OnRep_GridSize", Category = "Config")
	FIntPoint GridSize = FIntPoint(10, 40);

	UPROPERTY(EditAnywhere, Replicated, Category = "Data")
	FSpatialContent OccupiedSlots;

private:
	FSpatialEntryChangedNative SpatialEntryChangedDelegateNative;
	FGridSizeChangedNative GridSizeChangedDelegateNative;
};
