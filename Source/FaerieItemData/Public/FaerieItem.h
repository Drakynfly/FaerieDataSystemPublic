// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "FaerieItem.generated.h"

UENUM()
enum class EFaerieItemMutabilityFlags : uint8
{
	None,

	// Instance Mutability states that this instance is forever immutable because it lives in a package, or other outer,
	// that is static and cannot be changed, regardless of Token Mutability.
	InstanceMutability,

	// Token Mutability states that this item has one or more tokens that are allowed to mutate state.
	TokenMutability
};
ENUM_CLASS_FLAGS(EFaerieItemMutabilityFlags)

/**
 *
 */
UCLASS(DefaultToInstanced, EditInlineNew, BlueprintType)
class FAERIEITEMDATA_API UFaerieItem : public UObject
{
	GENERATED_BODY()

	friend class UFaerieItemToken;
	friend class UFaerieItemAsset;

public:
	virtual void PostLoad() override;
	virtual bool IsSupportedForNetworking() const override { return true; }
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Creates a new faerie item object. These are instance mutable by default.
	static UFaerieItem* CreateInstance();

	// Creates a new faerie item object using this instance as a template. Duplicates are instance mutable by default.
	UFaerieItem* CreateDuplicate() const;

	TArray<TObjectPtr<UFaerieItemToken>> GetTokens() const { return Tokens; }

	template <typename TFaerieItemToken>
	TFaerieItemToken* GetToken() const
	{
		static_assert(TIsDerivedFrom<TFaerieItemToken, UFaerieItemToken>::Value, TEXT("TFaerieItemToken is not derived from UFaerieItemToken"));
		return Cast<TFaerieItemToken>(GetToken(TFaerieItemToken::StaticClass()));
	}

	UFUNCTION(BlueprintCallable, Category = "FaerieItem", meta = (DeterminesOutputType = Class))
	UFaerieItemToken* GetToken(TSubclassOf<UFaerieItemToken> Class) const;

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "FaerieItem", meta = (DeterminesOutputType = Class, DynamicOutputParam = FoundToken, ExpandBoolAsExecs = ReturnValue))
	bool FindToken(TSubclassOf<UFaerieItemToken> Class, UFaerieItemToken*& FoundToken) const;

	UFUNCTION(BlueprintCallable, Category = "FaerieItem")
	void AddToken(UFaerieItemToken* Token);

	UFUNCTION(BlueprintCallable, Category = "FaerieItem")
	bool RemoveToken(UFaerieItemToken* Token);

	UFUNCTION(BlueprintCallable, Category = "FaerieItem")
	bool RemoveTokenByClass(TSubclassOf<UFaerieItemToken> Class);

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
};