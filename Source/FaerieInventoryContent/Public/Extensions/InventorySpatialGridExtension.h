// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "FaerieFastArraySerializerHack.h"
#include "ItemContainerExtensionBase.h"
#include "SpatialTypes.h"
#include "InventorySpatialGridExtension.generated.h"

class UInventorySpatialGridExtension;

USTRUCT(BlueprintType)
struct FSpatialItemPlacement
{
	GENERATED_BODY()

	FSpatialItemPlacement() = default;

	explicit FSpatialItemPlacement(const FIntPoint Origin)
	  : Origin(Origin) {}

	FSpatialItemPlacement(const FIntPoint Origin, const ESpatialItemRotation Rotation)
	  : Origin(Origin),
		Rotation(Rotation) {}

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "SpatialItemPlacement")
	FIntPoint Origin = FIntPoint::NoneValue;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "SpatialItemPlacement")
	ESpatialItemRotation Rotation = ESpatialItemRotation::None;

	friend bool operator==(const FSpatialItemPlacement& A, const FSpatialItemPlacement& B)
	{
		return A.Origin == B.Origin &&
			A.Rotation == B.Rotation;
	}

	friend bool operator<(const FSpatialItemPlacement& A, const FSpatialItemPlacement& B)
	{
		return A.Origin.X < B.Origin.X || (A.Origin.X == B.Origin.X && A.Origin.Y < B.Origin.Y);
	}
};

struct FSpatialContent;

USTRUCT(BlueprintType)
struct FSpatialKeyedStack : public FFastArraySerializerItem
{
	GENERATED_BODY()

	FSpatialKeyedStack() = default;

	FSpatialKeyedStack(const FInventoryKey Key, const FSpatialItemPlacement& Value)
	  : Key(Key), Value(Value) {}

	UPROPERTY(VisibleInstanceOnly, Category = "SpatialKeyedStack")
	FInventoryKey Key;

	UPROPERTY(VisibleInstanceOnly, Category = "SpatialKeyedStack")
	FSpatialItemPlacement Value;

	void PreReplicatedRemove(const FSpatialContent& InArraySerializer);
	void PostReplicatedAdd(FSpatialContent& InArraySerializer);
	void PostReplicatedChange(const FSpatialContent& InArraySerializer);
};

USTRUCT(BlueprintType)
struct FSpatialContent : public FFastArraySerializer,
						public TBinarySearchOptimizedArray<FSpatialContent, FSpatialKeyedStack>
{
	GENERATED_BODY()

	friend TBinarySearchOptimizedArray;
	friend UInventorySpatialGridExtension;

private:
	UPROPERTY(VisibleAnywhere, Category = "SpatialContent")
	TArray<FSpatialKeyedStack> Items;

	TArray<FSpatialKeyedStack>& GetArray() { return Items; }

	/** Owning storage to send Fast Array callbacks to */
	UPROPERTY()
	TWeakObjectPtr<UInventorySpatialGridExtension> ChangeListener;

public:
	bool EditItem(FInventoryKey Key, const TFunctionRef<bool(FSpatialItemPlacement&)>& Func);

	void PreStackReplicatedRemove(const FSpatialKeyedStack& Stack) const;
	void PostStackReplicatedAdd(const FSpatialKeyedStack& Stack);
	void PostStackReplicatedChange(const FSpatialKeyedStack& Stack) const;

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return Faerie::Hacks::FastArrayDeltaSerialize<FSpatialKeyedStack, FSpatialContent>(Items, DeltaParms, *this);
	}

	void Insert(FInventoryKey Key, const FSpatialItemPlacement& Value);

	void Remove(FInventoryKey Key);

	// Only const iteration is allowed.
	using TRangedForConstIterator = TArray<FSpatialKeyedStack>::RangedForConstIteratorType;
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

using FSpatialStackChangedNative = TMulticastDelegate<void(const FInventoryKey&, ESpatialEventType)>;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSpatialStackChanged, FInventoryKey, Key, ESpatialEventType, EventType);

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
	virtual EEventExtensionResponse AllowsAddition(const UFaerieItemContainerBase* Container, FFaerieItemStackView Stack, EFaerieStorageAddStackBehavior AddStackBehavior) override;
	virtual void PostAddition(const UFaerieItemContainerBase* Container, const Faerie::Inventory::FEventLog& Event) override;
	virtual void PostRemoval(const UFaerieItemContainerBase* Container, const Faerie::Inventory::FEventLog& Event) override;
	virtual void PostEntryChanged(const UFaerieItemContainerBase* Container, FEntryKey Key) override;
	//~ UItemContainerExtensionBase

	void PreStackRemove(const FSpatialKeyedStack& Stack);
	void PostStackAdd(const FSpatialKeyedStack& Stack);
	void PostStackChange(const FSpatialKeyedStack& Stack);

	UFUNCTION(/* Replication */)
	virtual void OnRep_GridSize();

private:
	bool AddItemToGrid(const FInventoryKey& Key, const UFaerieItem* Item);
	void RemoveItem(const FInventoryKey& Key, const UFaerieItem* Item);
	void RemoveItemBatch(const TConstArrayView<FInventoryKey>& Keys, const UFaerieItem* Item);

	// Gets a shape from a shape token on the item, or returns a single cell at 0,0 for items with no token.
	FFaerieGridShape GetItemShape_Impl(const UFaerieItem* Item) const;

public:
	bool CanAddItemToGrid(const FFaerieGridShapeConstView& Shape) const;

	bool MoveItem(const FInventoryKey& Key, const FIntPoint& TargetPoint);
	bool RotateItem(const FInventoryKey& Key);

	bool FitsInGrid(const FFaerieGridShapeConstView& Shape, const FSpatialItemPlacement& PlacementData, TConstArrayView<FInventoryKey> ExcludedKeys = {}, FIntPoint* OutCandidate = nullptr) const;

	bool FitsInGridAnyRotation(const FFaerieGridShapeConstView& Shape, FSpatialItemPlacement& PlacementData, TConstArrayView<FInventoryKey> ExcludedKeys = {}, FIntPoint* OutCandidate = nullptr) const;

	FSpatialItemPlacement FindFirstEmptyLocation(const FFaerieGridShapeConstView& Shape) const;

	FFaerieGridShape GetItemShape(FEntryKey Key) const;

	UFUNCTION(BlueprintCallable, Category = "Faerie|Grid")
	FSpatialItemPlacement GetStackPlacementData(const FInventoryKey& Key) const;

	UFUNCTION(BlueprintCallable, Category = "Faerie|Grid")
	FIntPoint GetStackBounds(const FInventoryKey& Key) const;

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Faerie|Grid")
	void SetGridSize(FIntPoint NewGridSize);

	FSpatialStackChangedNative::RegistrationType& GetOnSpatialStackChanged() { return SpatialStackChangedNative; }
	FGridSizeChangedNative::RegistrationType& GetOnGridSizeChanged() { return GridSizeChangedNative; }

protected:
	// Convert a point into a grid index
	int32 Ravel(const FIntPoint& Point) const;

	// Convert a grid index to a point
	FIntPoint Unravel(int32 Index) const;

	static FFaerieGridShape ApplyPlacement(const FFaerieGridShapeConstView& Shape, const FSpatialItemPlacement& Placement);

	// @todo Drakyn: look at these
	FSpatialKeyedStack* FindOverlappingItem(const FFaerieGridShapeConstView& TranslatedShape, const FInventoryKey& ExcludeKey);

	bool TrySwapItems(FInventoryKey KeyA, FSpatialItemPlacement& PlacementA, FInventoryKey KeyB, FSpatialItemPlacement& PlacementB);

	bool MoveSingleItem(const FInventoryKey Key, FSpatialItemPlacement& Placement, const FIntPoint& NewPosition);

	void UpdateItemPosition(const FInventoryKey Key, FSpatialItemPlacement& Placement, const FIntPoint& NewPosition);

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FSpatialStackChanged SpatialStackChangedDelegate;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FGridSizeChanged GridSizeChangedDelegate;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, ReplicatedUsing = "OnRep_GridSize", Category = "Config")
	FIntPoint GridSize = FIntPoint(10, 10);

	UPROPERTY(EditAnywhere, Replicated, Category = "Data")
	FSpatialContent SpatialEntries;

private:
	TBitArray<> OccupiedCells;

	FSpatialStackChangedNative SpatialStackChangedNative;
	FGridSizeChangedNative GridSizeChangedNative;

	/*
	 * @todo we do not support multiple containers. FSpatialContent would need to be refactored to allow that.
	 * (Or use UInventoryReplicatedDataExtensionBase). Until then, we can safely assume we only worry about one container.
	 * @todo2, additionally, this class is tied to UFaerieItemStorage. Initializing with other types isn't supported. Safeguard this?
	 */
	UPROPERTY(Replicated)
	TObjectPtr<UFaerieItemContainerBase> InitializedContainer;
};