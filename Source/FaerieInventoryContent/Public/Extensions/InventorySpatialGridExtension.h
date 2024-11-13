// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "FaerieFastArraySerializerHack.h"
#include "ItemContainerExtensionBase.h"
#include "SpatialTypes.h"
#include "InventorySpatialGridExtension.generated.h"

class UInventorySpatialGridExtension;
class UFaerieShapeToken;

USTRUCT(BlueprintType)
struct FSpatialItemPlacement
{
	GENERATED_BODY()

	FSpatialItemPlacement() : ItemShape(FFaerieGridShape::MakeSquare(1)) {}

	explicit FSpatialItemPlacement(const FFaerieGridShape& InShape)
		: ItemShape(InShape) {}

	FSpatialItemPlacement(const FFaerieGridShape& InShape,
						const FIntPoint Origin,
						const FIntPoint PivotPoint,
						const ESpatialItemRotation Rotation)
		: Origin(Origin)
		, PivotPoint(PivotPoint)
		, ItemShape(InShape)
		, Rotation(Rotation) {}

	FSpatialItemPlacement(const FFaerieGridShape& InShape,
						const FIntPoint Origin,
						const ESpatialItemRotation Rotation)
		: Origin(Origin)
		, ItemShape(InShape)
		, Rotation(Rotation)
	{
		PivotPoint = ItemShape.GetShapeCenter() + Origin;
	}

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "SpatialItemPlacement")
	FIntPoint Origin = FIntPoint::ZeroValue;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "SpatialItemPlacement")
	FIntPoint PivotPoint = FIntPoint::ZeroValue;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "SpatialItemPlacement")
	FFaerieGridShape ItemShape;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "SpatialItemPlacement")
	ESpatialItemRotation Rotation = ESpatialItemRotation::None;

	FFaerieGridShape GetRotated() const
	{
		return ItemShape.Rotate(Rotation);
	}

	friend bool operator==(const FSpatialItemPlacement& A, const FSpatialItemPlacement& B)
	{
		return A.Origin == B.Origin &&
			A.PivotPoint == B.PivotPoint &&
			A.ItemShape == B.ItemShape &&
			A.Rotation == B.Rotation;
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
	  : Key(Key), Value(Value) {}

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

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return Faerie::Hacks::FastArrayDeltaSerialize<FSpatialKeyedEntry, FSpatialContent>(Items, DeltaParms, *this);
	}

	void Insert(FInventoryKey Key, const FSpatialItemPlacement& Value);

	void Remove(FInventoryKey Key);

	// Only const iteration is allowed.
	using TRangedForConstIterator = TArray<FSpatialKeyedEntry>::RangedForConstIteratorType;
	FORCEINLINE TRangedForConstIterator begin() const { return TRangedForConstIterator(Items.begin()); }
	FORCEINLINE TRangedForConstIterator end() const { return TRangedForConstIterator(Items.end()); }
};

template <>
struct TStructOpsTypeTraits<FSpatialContent> : TStructOpsTypeTraitsBase2<FSpatialContent>
{
	enum
	{
		WithNetDeltaSerializer = true,
	};
};

UENUM(BlueprintType)
enum class ESpatialEventType : uint8
{
	ItemAdded,
	ItemChanged,
	ItemRemoved
};

using FSpatialEntryChangedNative = TMulticastDelegate<void(const FInventoryKey&, ESpatialEventType)>;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSpatialEntryChanged, FInventoryKey, EntryKey, ESpatialEventType, EventType);

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
	virtual EEventExtensionResponse AllowsAddition(const UFaerieItemContainerBase* Container, FFaerieItemStackView Stack) override;
	virtual void PostAddition(const UFaerieItemContainerBase* Container, const Faerie::Inventory::FEventLog& Event) override;
	virtual void PostRemoval(const UFaerieItemContainerBase* Container, const Faerie::Inventory::FEventLog& Event) override;
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
	bool CanAddItemToGrid(const UFaerieShapeToken* ShapeToken) const;

	bool MoveItem(const FInventoryKey& Key, const FIntPoint& SourcePoint, const FIntPoint& TargetPoint);
	bool RotateItem(const FInventoryKey& Key);

	// @todo probably split into two functions. one with rotation check, one without. public API probably doesn't need to see the rotation check!
	bool FitsInGrid(const FSpatialItemPlacement& PlacementData, TConstArrayView<FInventoryKey> ExcludedKeys = {}, FIntPoint* OutCandidate = nullptr) const;

	void FindFirstEmptyLocation(FSpatialItemPlacement& OutPlacementData) const;

	UFUNCTION(BlueprintCallable, Category = "Grid")
	FSpatialItemPlacement GetEntryPlacementData(const FInventoryKey& Key) const;

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Grid")
	void SetGridSize(FIntPoint NewGridSize);

	FSpatialEntryChangedNative::RegistrationType& GetOnSpatialEntryChanged() { return SpatialEntryChangedDelegateNative; }
	FGridSizeChangedNative::RegistrationType& GetOnGridSizeChanged() { return GridSizeChangedDelegateNative; }

protected:
	FSpatialKeyedEntry* FindItemByKey(const FInventoryKey& Key);

	// @todo Drakyn: look at these
	FSpatialKeyedEntry* FindOverlappingItem(const FFaerieGridShape& Shape, const FIntPoint& Offset,
											const FInventoryKey& ExcludeKey);
	bool TrySwapItems(FSpatialKeyedEntry& MovingItem, FSpatialKeyedEntry& OverlappingItem, const FIntPoint& Offset);
	bool MoveSingleItem(FSpatialKeyedEntry& Item, const FIntPoint& Offset);
	void UpdateItemPosition(FSpatialKeyedEntry& Item, const FIntPoint& Offset);

	UFUNCTION(BlueprintCallable, Category = "Grid")
	FIntPoint GetEntryBounds(const FInventoryKey& Entry) const;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FSpatialEntryChanged SpatialEntryChangedDelegate;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FGridSizeChanged GridSizeChangedDelegate;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, ReplicatedUsing = "OnRep_GridSize", Category = "Config")
	FIntPoint GridSize = FIntPoint(10, 10);

	UPROPERTY(EditAnywhere, Replicated, Category = "Data")
	FSpatialContent SpatialEntries;

private:
	TBitArray<> OccupiedCells;

	FSpatialEntryChangedNative SpatialEntryChangedDelegateNative;
	FGridSizeChangedNative GridSizeChangedDelegateNative;
};