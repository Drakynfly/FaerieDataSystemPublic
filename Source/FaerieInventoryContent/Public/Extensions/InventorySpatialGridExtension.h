// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "ItemContainerExtensionBase.h"
#include "InventorySpatialGridExtension.generated.h"

class UInventorySpatialGridExtension;
class UFaerieShapeToken;


USTRUCT(BlueprintType)
struct FFaerieGridShape
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly, Category = "Grid")
	TArray<FIntPoint> Points;
};

USTRUCT()
struct FSpatialEntryKey
{
	GENERATED_BODY()
	UPROPERTY(VisibleInstanceOnly, Category = "Grid")
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

	FSpatialKeyedEntry();

	FSpatialKeyedEntry(const FSpatialEntryKey Key, const FEntryKey Value) : Key(Key), Value(Value)
	{
	}

	UPROPERTY(VisibleInstanceOnly, Category = "Grid")
	FSpatialEntryKey Key;

	UPROPERTY(VisibleInstanceOnly, Category = "Grid")
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
	UPROPERTY(VisibleAnywhere, Category = "Content")
	TArray<FSpatialKeyedEntry> Items;

	TArray<FSpatialKeyedEntry>& GetArray() { return Items; }

	/** Owning storage to send Fast Array callbacks to */
	UPROPERTY()
	TWeakObjectPtr<UInventorySpatialGridExtension> ChangeListener;

public:
	const TArray<FSpatialKeyedEntry>& GetEntries() const { return Items; };

	void PreEntryReplicatedRemove(const FSpatialKeyedEntry& Entry) const;
	void PostEntryReplicatedAdd(const FSpatialKeyedEntry& Entry);
	void PostEntryReplicatedChange(const FSpatialKeyedEntry& Entry) const;

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams)
	{
		return FastArrayDeltaSerialize<FSpatialKeyedEntry, FSpatialContent>(Items, DeltaParams, *this);
	}

	void Insert(FSpatialEntryKey Value, const FEntryKey& Key);

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

/**
 * 
 */
UCLASS()
class FAERIEINVENTORYCONTENT_API UInventorySpatialGridExtension : public UItemContainerExtensionBase
{
	friend FSpatialContent;
	GENERATED_BODY()

public:
	FSpatialEntryChangedNative& GetOnSpatialEntryChangedNative() { return SpatialEntryChangedDelegateNative; }

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	bool CanAddItemToGrid(const UFaerieShapeToken* ShapeToken, const FIntPoint& Position) const;
	bool CanAddItemToGrid(const UFaerieShapeToken* ShapeToken) const;
	bool AddItemToGrid(const FEntryKey& Key, const UFaerieShapeToken* ShapeToken, const FIntPoint& Position);
	void RemoveItemFromGrid(const FEntryKey& Key);
	virtual void PostInitProperties() override;
	UFUNCTION(BlueprintCallable, Category = "Grid")
	FFaerieGridShape GetEntryPositions(UPARAM() const FEntryKey& Key) const;

	FSpatialContent& GetContent() { return OccupiedSlots; }

protected:
	virtual EEventExtensionResponse AllowsAddition(const UFaerieItemContainerBase* Container,
	                                               FFaerieItemStackView Stack) override;
	virtual void PostAddition(const UFaerieItemContainerBase* Container,
	                          const Faerie::Inventory::FEventLog& Event) override;
	virtual void PostRemoval(const UFaerieItemContainerBase* Container,
	                         const Faerie::Inventory::FEventLog& Event) override;

	UPROPERTY(BlueprintAssignable, Category = "Delegates")
	FSpatialEntryChanged SpatialEntryChangedDelegate;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
	FIntPoint GridSize = FIntPoint(10, 40);

	UPROPERTY(EditAnywhere, Replicated, Category = "Data")
	FSpatialContent OccupiedSlots;

private:
	FSpatialEntryChangedNative SpatialEntryChangedDelegateNative;
};
