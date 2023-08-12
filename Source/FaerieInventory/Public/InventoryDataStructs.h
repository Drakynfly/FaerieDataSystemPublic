// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "FaerieItem.h"
#include "FaerieItemStack.h"
#include "GameplayTagContainer.h"
#include "TTypedTagStaticImpl2.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "InventoryDataStructs.generated.h"

/**
 * This enum hold the flags to bitwise equivalate inventory entries.
 */
UENUM(BlueprintType, Meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor))
enum class EEntryEquivelancyFlags : uint8
{
	None = 0 UMETA(Hidden),

	// When set, all generated data will be checked.
	ItemData = 1 << 0,

	// When set, stack sums will be checked.
	StackSum = 1 << 1,

	// When set, stack limit will be checked.
	Limit = 1 << 2,

	// When set, all other flags are considered set.
	All = ItemData | StackSum | Limit UMETA(Hidden),
};
ENUM_CLASS_FLAGS(EEntryEquivelancyFlags)

DECLARE_LOG_CATEGORY_EXTERN(LogInventoryStructs, Log, All)

LLM_DECLARE_TAG(ItemStorage);

/**
 * A unique key that maps to a faerie item in some way that is persistent across the network and play-sessions.
 * Usually only guaranteed to be unique per implementing container.
 * Moving an item between containers will not preserve the key.
 */
USTRUCT(BlueprintType)
struct FFaerieItemKeyBase
{
	GENERATED_BODY()

	FFaerieItemKeyBase()
	{
		KeyValue = INDEX_NONE;
	}

	FFaerieItemKeyBase(const int32 Value)
	{
		KeyValue = Value;
	}

private:
	UPROPERTY(VisibleAnywhere, Category = "FaerieItemKeyBase")
	int32 KeyValue;

public:

	bool IsValid() const
	{
		return KeyValue > INDEX_NONE;
	}

	friend bool operator==(const FFaerieItemKeyBase& Lhs, const FFaerieItemKeyBase& Rhs)
	{
		return Lhs.KeyValue == Rhs.KeyValue;
	}

	friend bool operator!=(const FFaerieItemKeyBase& Lhs, const FFaerieItemKeyBase& Rhs)
	{
		return !(Lhs == Rhs);
	}

	FORCEINLINE friend uint32 GetTypeHash(const FFaerieItemKeyBase& Key)
	{
		return Key.KeyValue;
	}

	struct FCompare
    {
    	FORCEINLINE bool operator()(const FFaerieItemKeyBase& A, const FFaerieItemKeyBase& B) const
        {
    		return A.KeyValue < B.KeyValue;
    	}
    };

	int32 Value() const { return KeyValue; }

	/** Get internal value as string for debugging */
	FString ToString() const { return FString::FromInt(KeyValue); }

	friend FArchive& operator<<(FArchive& Ar, FFaerieItemKeyBase& Val)
	{
		return Ar << Val.KeyValue;
	}
};

USTRUCT(BlueprintType)
struct FAERIEINVENTORY_API FEntryKey : public FFaerieItemKeyBase
{
	GENERATED_BODY()
	using FFaerieItemKeyBase::FFaerieItemKeyBase;

	static FEntryKey InvalidKey;
};

USTRUCT(BlueprintType)
struct FAERIEINVENTORY_API FStackKey : public FFaerieItemKeyBase
{
	GENERATED_BODY()
	using FFaerieItemKeyBase::FFaerieItemKeyBase;
};

// ReSharper disable CppUE4CodingStandardNamingViolationWarning
// These will just be compiled out anyway, they exist to give meaning to certain keywords, so numbers can be inserted
// and preserve their meaning when read.
enum { INVENTORY_MAXIMUM_SLOTS = 32767 };
enum { INVENTORY_MAXIMUM_ITEM_COUNT = 32767 };

USTRUCT(BlueprintType)
struct FAERIEINVENTORY_API FInventoryStack
{
	GENERATED_BODY()

	FInventoryStack() {}

	FInventoryStack(const int32 Amount)
	  : Amount(Amount)
	{
		check(Amount >= 0);
	}

protected:
	// Amount in the stack
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InventoryStack", meta = (UIMin = 0, ClampMin = -1))
	int32 Amount = 0;

public:
	int32 GetAmount() const { return Amount; }

	bool IsValid() const;

	friend bool operator==(const FInventoryStack& Lhs, const FInventoryStack& Rhs)
	{
		return Lhs.Amount == Rhs.Amount;
	}

	friend bool operator!=(const FInventoryStack& Lhs, const FInventoryStack& Rhs)
	{
		return !(Lhs == Rhs);
	}

	void operator+=(const FInventoryStack& Other)
	{
		Amount += Other.Amount;
	}

	void operator-=(const FInventoryStack& Other)
	{
		check(Amount >= Other.Amount);
		Amount -= Other.Amount;
	}

	friend FInventoryStack operator+(const FInventoryStack& A, const FInventoryStack& B)
	{
		return A.Amount + B.Amount;
	}

	friend FInventoryStack operator-(const FInventoryStack& A, const FInventoryStack& B)
	{
		check(A >= B);
		return A.Amount - B.Amount;
	}

	bool operator<(const FInventoryStack& Other) const
	{
		return Amount < Other.Amount;
	}

	bool operator<=(const FInventoryStack& Other) const
	{
		return Amount <= Other.Amount;
	}

	bool operator>(const FInventoryStack& Other) const
	{
		return Amount > Other.Amount;
	}

	bool operator>=(const FInventoryStack& Other) const
	{
		return Amount >= Other.Amount;
	}

	static int32 EmptyStackNum;
	static int32 UnlimitedNum;
	static FInventoryStack EmptyStack;
	static FInventoryStack UnlimitedStack;

private:
	static FInventoryStack MakeUnlimitedStack();
};

USTRUCT(BlueprintType)
struct FAERIEINVENTORY_API FKeyedStack
{
	GENERATED_BODY()

	// Unique key to identify this stack.
	UPROPERTY(VisibleAnywhere, Category = "Keyed Stack")
	FStackKey Key;

	// Amount in the stack
	UPROPERTY(VisibleAnywhere, Category = "Keyed Stack")
	FInventoryStack Stack;

	friend bool operator==(const FKeyedStack& Lhs, const FStackKey& Rhs)
	{
		return Lhs.Key == Rhs;
	}

	friend bool operator==(const FKeyedStack& Lhs, const FKeyedStack& Rhs)
	{
		return Lhs.Key == Rhs.Key
			   && Lhs.Stack == Rhs.Stack;
	}

	friend bool operator!=(const FKeyedStack& Lhs, const FKeyedStack& Rhs)
	{
		return !(Lhs == Rhs);
	}
};


/**
 * An entry key and stack key pair. This is a true "key" to a exact inventory stack as seen by an external interface.
 */
USTRUCT(BlueprintType)
struct FInventoryKey
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "InventoryKey")
	FEntryKey EntryKey;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "InventoryKey")
	FStackKey StackKey;

	FORCEINLINE friend uint32 GetTypeHash(const FInventoryKey Key)
	{
		return HashCombine(GetTypeHash(Key.EntryKey), GetTypeHash(Key.StackKey));
	}

	bool IsValid() const { return EntryKey.IsValid() && StackKey.IsValid(); }

	FString ToString() const
	{
		return EntryKey.ToString() + ":" + StackKey.ToString();
	}

	friend bool operator==(const FInventoryKey Lhs, const FInventoryKey Rhs)
	{
		return Lhs.EntryKey == Rhs.EntryKey
			   && Lhs.StackKey == Rhs.StackKey;
	}

	friend bool operator!=(const FInventoryKey Lhs, const FInventoryKey Rhs)
	{
		return !(Lhs == Rhs);
	}
};

/**
 * The key used to flag entries with custom data.
 */
USTRUCT(BlueprintType, meta = (Categories = "Fae.Inventory"))
struct FFaerieInventoryTag : public FGameplayTag
{
	GENERATED_BODY()

	FFaerieInventoryTag() {}
	static FFaerieInventoryTag GetRootTag() { return TTypedTagStaticImpl2<FFaerieInventoryTag>::StaticImpl.RootTag; }
	static FFaerieInventoryTag TryConvert(const FGameplayTag FromTag) { return TTypedTagStaticImpl2<FFaerieInventoryTag>::TryConvert(FromTag, false); }
	static FFaerieInventoryTag ConvertChecked(const FGameplayTag FromTag) { return TTypedTagStaticImpl2<FFaerieInventoryTag>::TryConvert(FromTag, true); }
	static FFaerieInventoryTag AddNativeTag(const FString& TagBody, const FString& DevComment) { return TTypedTagStaticImpl2<FFaerieInventoryTag>::AddNativeTag(TagBody, DevComment); }
	bool ExportTextItem(FString& ValueStr, const FFaerieInventoryTag& DefaultValue, UObject* Parent, const int32 PortFlags, UObject* ExportRootScope) const
	{
		return TTypedTagStaticImpl2<FFaerieInventoryTag>::ExportTextItem(*this, ValueStr, PortFlags);
	}

protected:
	FFaerieInventoryTag(const FGameplayTag& Tag) { TagName = Tag.GetTagName(); }
	static const TCHAR* GetRootTagStr() { return TEXT("Fae.Inventory"); }
	friend class TTypedTagStaticImpl2<FFaerieInventoryTag>;
};

template<> struct TNameOf<FFaerieInventoryTag> { FORCEINLINE static TCHAR const* GetName() { return TEXT("FFaerieInventoryTag"); } };

template<>
struct TStructOpsTypeTraits<FFaerieInventoryTag> : public TStructOpsTypeTraitsBase2<FFaerieInventoryTag>
{
	enum
	{
		WithExportTextItem = true,
		WithImportTextItem = true
	};
};

class UFaerieItem;

/**
 * The struct for containing one inventory entry.
*/
USTRUCT(BlueprintType)
struct FAERIEINVENTORY_API FInventoryEntry
{
    GENERATED_BODY()

    FInventoryEntry() {}

    // Actual data for this entry.
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "InventoryEntry")
    TObjectPtr<UFaerieItem> ItemObject;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "InventoryEntry")
	TArray<FKeyedStack> Stacks;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "InventoryEntry")
	FInventoryStack Limit = 0;

private:
	// Internal count of how many stacks we've made.
	int32 StackCount = 0;

public:
	FInventoryStack GetStack(const FStackKey& Key) const;

	TArray<FStackKey> GetKeys() const;

	FInventoryStack StackSum() const;

	void Set(const FStackKey& Key, const FInventoryStack Stack);

	// Add the Amount to the stacks, adding new stacks as needed. Can optionally return the list of added stacks.
	// ReturnValue is FInventoryStack::Empty if Stack was successfully added, or the remainder, if not.
	FInventoryStack AddToAnyStack(FInventoryStack Stack, TArray<FStackKey>* OutAddedKeys = nullptr);

	// Remove the amount from any number of stacks. Can optionally return the list of modified stacks, and/or just the removed stacks
	// ReturnValue is 0 if Stack was successfully removed, or the remainder, if not.
	FInventoryStack RemoveFromAnyStack(FInventoryStack Amount, TArray<FStackKey>* OutAllModifiedKeys = nullptr, TArray<FStackKey>* OutRemovedKeys = nullptr);

	bool IsValid() const;

    FFaerieItemStack ToItemStack() const;

    static bool IsEqualTo(const FInventoryEntry& A, const FInventoryEntry& B, EEntryEquivelancyFlags CheckFlags);
};


struct FInventoryContent;

// A minimal struct to replicate a Key and Value pair as a emulation of a TMap
USTRUCT()
struct FKeyedInventoryEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()

	FKeyedInventoryEntry() {}

	FKeyedInventoryEntry(const FEntryKey Key, const FInventoryEntry& Entry)
		: Key(Key),
		  Entry(Entry)
	{
	}

	// Unique key to identify this entry.
	UPROPERTY(VisibleAnywhere, Category = "Inventory Key and Entry")
	FEntryKey Key;

	// A canonical entry.
	UPROPERTY(VisibleAnywhere, Category = "Inventory Key and Entry")
	FInventoryEntry Entry;

	void PreReplicatedRemove(const FInventoryContent& InArraySerializer);
	void PostReplicatedAdd(const FInventoryContent& InArraySerializer);
	void PostReplicatedChange(const FInventoryContent& InArraySerializer);
};

/**
 * FInventoryContent is a Fast Array, containing all FInventoryEntries for an inventory. Lookup is O(Log(n)), as FInventoryKeys
 * are used to keep Items in numeric order, allowing for binary searches on getter.
 */
USTRUCT()
struct FInventoryContent : public FFastArraySerializer
{
	GENERATED_BODY()

	friend class UFaerieItemStorage;

private:
	UPROPERTY(VisibleAnywhere, Category = "Content")
	TArray<FKeyedInventoryEntry> Items;

	/** Owning inventory to send Fast Array callbacks to */
	UPROPERTY()
	TWeakObjectPtr<UFaerieItemStorage> ChangeListener;

private:
	int32 IndexOf(const FEntryKey Key) const;

public:
	bool Contains(FEntryKey Key) const;

	const FInventoryEntry* Find(FEntryKey Key) const;

	const FInventoryEntry& operator[](const FEntryKey Key) const;

	FEntryKey GetKeyAt(int32 Index) const;

	/**
	 * Adds a new key and entry to the end of the Items array. Performs a quick check that the new key is sequentially
	 * following the end of the array, but does not enforce or check for the entire array being sorted. Use this function
	 * when you can confirm that the key is sequential. When this is not known, use Insert instead. Append is O(1), while
	 * Insert is O(Log(n)), so use this if you can.
	 * */
	FKeyedInventoryEntry& Append(FEntryKey Key, const FInventoryEntry& Entry);

	/**
	 * Performs a binary search to find where to insert this new key. Needed when Key is not guaranteed to be sequential.
	 * @see Append
	 */
	void Insert(FEntryKey Key, const FInventoryEntry& Entry);

	/**
	 * Force a full resort of the contained items. Must be called whenever there are changes made to the array without
	 * verifying sort order. Example use case is mass addition of items of unknown order. Instead of sorting each one
	 * as they are added, it's more efficient to add them all in whatever order they came in, or perform one sort after.
	 */
	void Sort();

	void Remove(FEntryKey Key);

	bool IsEmpty() const { return Items.IsEmpty(); }

	int32 Num() const { return Items.Num(); }

	void GetKeys(TArray<FEntryKey>& Array) const
	{
		for (const FKeyedInventoryEntry& Item : Items)
		{
			Array.Add(Item.Key);
		}
	}

	struct FScopedItemHandle
	{
		FScopedItemHandle(const FEntryKey Key, FInventoryContent& Source)
		  : Handle(Source.Items[Source.IndexOf(Key)]),
			Source(Source) {}

		~FScopedItemHandle();

	protected:
		FKeyedInventoryEntry& Handle;

	private:
		FInventoryContent& Source;

	public:
		FInventoryEntry* operator->() const { return &Handle.Entry; }
		FInventoryEntry& Get() const { return Handle.Entry; }
	};

	FScopedItemHandle GetMutableHandle(const FEntryKey Key)
	{
		return FScopedItemHandle(Key, *this);
	}

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FastArrayDeltaSerialize<FKeyedInventoryEntry, FInventoryContent>(Items, DeltaParms, *this);
	}

	/*
	void PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize) const;
	void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize) const;
	void PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize) const;
	*/

	void PreEntryReplicatedRemove(const FKeyedInventoryEntry& Entry) const;
	void PostEntryReplicatedAdd(const FKeyedInventoryEntry& Entry) const;
	void PostEntryReplicatedChange(const FKeyedInventoryEntry& Entry) const;

	// Only const iteration is allowed.
	using TRangedForConstIterator = TArray<FKeyedInventoryEntry>::RangedForConstIteratorType;
	FORCEINLINE TRangedForConstIterator begin() const { return TRangedForConstIterator(Items.begin()); }
	FORCEINLINE TRangedForConstIterator end() const   { return TRangedForConstIterator(Items.end());   }
};

template<>
struct TStructOpsTypeTraits<FInventoryContent> : public TStructOpsTypeTraitsBase2<FInventoryContent>
{
	enum
	{
		WithNetDeltaSerializer = true,
	};
};

/**
 *
 */
USTRUCT(BlueprintType)
struct FAERIEINVENTORY_API FInventoryKeyHandle
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TWeakObjectPtr<class UFaerieItemStorage> ItemStorage;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FInventoryKey Key;
};

namespace Faerie::Inventory
{
	class FAERIEINVENTORY_API Utils
	{
	public:
		static void BreakKeyedEntriesIntoInventoryKeys(const TArray<FKeyedInventoryEntry>& Entries, TArray<FInventoryKey>& OutKeys);
	};
}
