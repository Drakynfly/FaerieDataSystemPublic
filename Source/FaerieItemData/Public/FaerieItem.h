// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "FaerieItemToken.h"

#include "FaerieItem.generated.h"

UENUM()
enum class EFaerieItemMutabilityFlags : uint8
{
	None,

	// 'Instance Mutability' is a required flag for item instances to be changed after construction. Without this flag,
	// items are understood to be forever immutable because it either lives in a package, or other outer that is static
	// and itself cannot be changed, regardless of whether the instance has Token Mutability enabled.
	// Items created at runtime or duplicated from static instances have this flag enabled by default.
	// (See the definitions of CreateInstance / CreateDuplicate)
	InstanceMutability,

	// 'Token Mutability' says that this item has one or more tokens that request the ability to mutate their internal state.
	TokenMutability
};
ENUM_CLASS_FLAGS(EFaerieItemMutabilityFlags)

using FNotifyOwnerOfSelfMutation = TDelegate<void(const class UFaerieItem*, const class UFaerieItemToken*)>;

/**
 * An runtime instance of an item.
 */
UCLASS(DefaultToInstanced, EditInlineNew, BlueprintType)
class FAERIEITEMDATA_API UFaerieItem : public UObject
{
	GENERATED_BODY()

	friend class UFaerieItemToken;
	friend class UFaerieItemAsset;

public:
	virtual void PreSave(FObjectPreSaveContext SaveContext) override;
	virtual void PostLoad() override;
	virtual bool IsSupportedForNetworking() const override { return true; }
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Iterates over each contained token. Return true in the delegate to continue iterating.
	void ForEachToken(const TFunction<bool(const UFaerieItemToken*)>& Iter) const;

	// Iterates over each contained token. Return true in the delegate to continue iterating.
	void ForEachTokenOfClass(const TFunction<bool(const UFaerieItemToken*)>& Iter, TSubclassOf<UFaerieItemToken> Class) const;

	// Iterates over each contained token. Return true in the delegate to continue iterating.
	template <typename TFaerieItemTokenClass>
	void ForEachToken(const TFunction<bool(const TFaerieItemTokenClass*)>& Iter) const
	{
		for (auto&& Token : Tokens)
		{
			if (IsValid(Token) && Token->IsA<TFaerieItemTokenClass>())
			{
				if (!Iter(Cast<TFaerieItemTokenClass>(Token)))
				{
					return;
				}
			}
		}
	}

protected:
		  UFaerieItemToken* GetTokenImpl(TSubclassOf<UFaerieItemToken> Class);
	const UFaerieItemToken* GetTokenImpl(TSubclassOf<UFaerieItemToken> Class) const;

public:
	// Creates a new faerie item object. These are instance mutable by default.
	static UFaerieItem* CreateInstance();

	// Creates a new faerie item object using this instance as a template. Duplicates are instance mutable by default.
	UFaerieItem* CreateDuplicate() const;

	TConstArrayView<TObjectPtr<UFaerieItemToken>> GetTokens() const { return Tokens; }

	template <typename TFaerieItemToken>
	const TFaerieItemToken* GetToken() const
	{
		static_assert(TIsDerivedFrom<TFaerieItemToken, UFaerieItemToken>::Value, TEXT("TFaerieItemToken must derived from UFaerieItemToken"));
		return Cast<TFaerieItemToken>(GetTokenImpl(TFaerieItemToken::StaticClass()));
	}

	template <typename TFaerieItemToken>
	TFaerieItemToken* GetEditableToken()
	{
		static_assert(TIsDerivedFrom<TFaerieItemToken, UFaerieItemToken>::Value, TEXT("TFaerieItemToken must derived from UFaerieItemToken"));
		return Cast<TFaerieItemToken>(GetTokenImpl(TFaerieItemToken::StaticClass()));
	}

	// @todo this isn't const safe
	UFUNCTION(BlueprintCallable, Category = "FaerieItem", meta = (DeterminesOutputType = Class))
	UFaerieItemToken* GetToken(TSubclassOf<UFaerieItemToken> Class) const;

	// @todo this isn't const safe
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "FaerieItem", meta = (DeterminesOutputType = Class, DynamicOutputParam = FoundToken, ExpandBoolAsExecs = ReturnValue))
	bool FindToken(TSubclassOf<UFaerieItemToken> Class, UFaerieItemToken*& FoundToken) const;

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "FaerieItem", meta = (DeterminesOutputType = Class, DynamicOutputParam = FoundTokens))
	void FindTokens(TSubclassOf<UFaerieItemToken> Class, TArray<UFaerieItemToken*>& FoundTokens) const;

	UFUNCTION(BlueprintCallable, Category = "FaerieItem")
	void AddToken(UFaerieItemToken* Token);

	UFUNCTION(BlueprintCallable, Category = "FaerieItem")
	bool RemoveToken(UFaerieItemToken* Token);

	UFUNCTION(BlueprintCallable, Category = "FaerieItem")
	int32 RemoveTokensByClass(TSubclassOf<UFaerieItemToken> Class);

	UFUNCTION(BlueprintCallable, Category = "FaerieItem")
	FDateTime GetLastModified() const { return LastModified; }

	// Can this item object be changed whatsoever at runtime. This is not available for static or precached items.
	UFUNCTION(BlueprintCallable, Category = "FaerieItem")
	bool IsInstanceMutable() const;

	// Can the data in the tokens of this item change at runtime. Only true if also instance mutable.
	UFUNCTION(BlueprintCallable, Category = "FaerieItem")
	bool IsDataMutable() const;

protected:
	void OnTokenEdited(UFaerieItemToken* Token);

	void CacheTokenMutability();

public:
	FNotifyOwnerOfSelfMutation& GetNotifyOwnerOfSelfMutation() { return NotifyOwnerOfSelfMutation; }

protected:
	UPROPERTY(Replicated, VisibleInstanceOnly, Category = "FaerieItem")
	TArray<TObjectPtr<UFaerieItemToken>> Tokens;

	// Keeps track of the last time this item was modified. Allows, for example, sorting items by recently touched.
	UPROPERTY(Replicated, VisibleInstanceOnly, Category = "FaerieItem")
	FDateTime LastModified = FDateTime();

	// Mutability flags. In order for an item instance to be changed at runtime it must have no mutually exclusive
	// flags.
	UPROPERTY(Replicated, VisibleInstanceOnly, Category = "FaerieItem")
	EFaerieItemMutabilityFlags MutabilityFlags;

	// Delegate for owners to bind to, for detecting when tokens are mutated outside of their knowledge
	FNotifyOwnerOfSelfMutation NotifyOwnerOfSelfMutation;
};