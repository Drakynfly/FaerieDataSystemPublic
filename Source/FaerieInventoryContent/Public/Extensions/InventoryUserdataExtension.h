// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "GameplayTagContainer.h"
#include "ItemContainerExtensionBase.h"
#include "InventoryReplicatedDataExtensionBase.h"
#include "ActorClasses/FaerieInventoryClient.h"
#include "InventoryUserdataExtension.generated.h"

/**
 * The key used to tag entries with custom client data.
 */
USTRUCT(BlueprintType, meta = (Categories = "Fae.Inventory.Public"))
struct FAERIEINVENTORYCONTENT_API FFaerieInventoryUserTag : public FFaerieInventoryTag
{
	GENERATED_BODY()
	END_TAG_DECL2(FFaerieInventoryUserTag, TEXT("Fae.Inventory.Public"))
};

struct FAERIEINVENTORYCONTENT_API FFaerieInventoryUserTags : public FGameplayTagNativeAdder
{
	FORCEINLINE static const FFaerieInventoryUserTags& Get() { return FaerieInventoryUserTags; }

	FFaerieInventoryUserTag Favorite;

protected:
	virtual void AddTags() override
	{
		Favorite = FFaerieInventoryUserTag::AddNativeTag(TEXT("Favorite"),
							"Marks an item to show up in player favorites / quick access.");
	}

private:
	// Private static object for the global tags. Use the Get() function to access externally.
	static FFaerieInventoryUserTags FaerieInventoryUserTags;
};

USTRUCT()
struct FInventoryEntryUserdata
{
	GENERATED_BODY()

	FInventoryEntryUserdata() {}

	FInventoryEntryUserdata(const FGameplayTagContainer& Tags)
	  : Tags(Tags) {}

	UPROPERTY(EditAnywhere, Category = "InventoryEntryUserdata", meta = (Categories = "Fae.Inventory.Public"))
	FGameplayTagContainer Tags;

	friend bool operator==(const FInventoryEntryUserdata& Lhs, const FInventoryEntryUserdata& Rhs)
	{
		return Lhs.Tags == Rhs.Tags;
	}

	friend bool operator!=(const FInventoryEntryUserdata& Lhs, const FInventoryEntryUserdata& Rhs)
	{
		return !(Lhs == Rhs);
	}
};

/*
 * An extension added to player inventories that stores additional userdata about items, such as favorites.
 */
UCLASS()
class FAERIEINVENTORYCONTENT_API UInventoryUserdataExtension : public UInventoryReplicatedDataExtensionBase
{
	GENERATED_BODY()

protected:
	//~ UInventoryReplicatedDataExtensionBase
	virtual UScriptStruct* GetDataScriptStruct() const override;
	//~ UInventoryReplicatedDataExtensionBase

public:
	UFUNCTION(BlueprintCallable, Category = "Faerie|UserdataExtension")
	bool DoesStackHaveTag(UFaerieItemContainerBase* Container, FEntryKey Key, FFaerieInventoryUserTag Tag) const;

	UFUNCTION(BlueprintCallable, Category = "Faerie|UserdataExtension")
	bool CanSetStackTag(UFaerieItemContainerBase* Container, FEntryKey Key, FFaerieInventoryUserTag Tag, const bool StateToSetTo) const;

	bool MarkStackWithTag(UFaerieItemContainerBase* Container, FEntryKey Key, FFaerieInventoryUserTag Tag);

	bool ClearTagFromStack(UFaerieItemContainerBase* Container, FEntryKey Key, FFaerieInventoryUserTag Tag);
};

USTRUCT(BlueprintType)
struct FFaerieClientAction_RequestMarkStackWithTag : public FFaerieClientActionBase
{
	GENERATED_BODY()

	virtual void Server_Execute(const UFaerieInventoryClient* Client) const override;

	UPROPERTY(BlueprintReadWrite, Category = "FaerieClientAction_RequestMarkStackWithTag")
	FInventoryKeyHandle Handle;

	UPROPERTY(BlueprintReadWrite, Category = "FaerieClientAction_RequestMarkStackWithTag")
	FFaerieInventoryUserTag Tag;
};

USTRUCT(BlueprintType)
struct FFaerieClientAction_RequestClearTagFromStack : public FFaerieClientActionBase
{
	GENERATED_BODY()

	virtual void Server_Execute(const UFaerieInventoryClient* Client) const override;

	UPROPERTY(BlueprintReadWrite, Category = "FaerieClientAction_RequestClearTagFromStack")
	FInventoryKeyHandle Handle;

	UPROPERTY(BlueprintReadWrite, Category = "FaerieClientAction_RequestClearTagFromStack")
	FFaerieInventoryUserTag Tag;
};