// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "GameplayTagContainer.h"
#include "InventoryExtensionBase.h"
#include "InventoryUserdataExtension.generated.h"

/**
 * The key used to tag entries with custom client data.
 */
USTRUCT(BlueprintType, meta = (Categories = "Fae.Inventory.Public"))
struct FAERIEINVENTORYCONTENT_API FFaerieInventoryUserTag : public FFaerieInventoryTag
{
	GENERATED_BODY()

	FFaerieInventoryUserTag() {}
	static FFaerieInventoryUserTag GetRootTag() { return TTypedTagStaticImpl2<FFaerieInventoryUserTag>::StaticImpl.RootTag; }
	static FFaerieInventoryUserTag TryConvert(const FGameplayTag FromTag) { return TTypedTagStaticImpl2<FFaerieInventoryUserTag>::TryConvert(FromTag, false); }
	static FFaerieInventoryUserTag ConvertChecked(const FGameplayTag FromTag) { return TTypedTagStaticImpl2<FFaerieInventoryUserTag>::TryConvert(FromTag, true); }
	static FFaerieInventoryUserTag AddNativeTag(const FString& TagBody, const FString& DevComment) { return TTypedTagStaticImpl2<FFaerieInventoryUserTag>::AddNativeTag(TagBody, DevComment); }
	bool ExportTextItem(FString& ValueStr, const FFaerieInventoryUserTag& DefaultValue, UObject* Parent, const int32 PortFlags, UObject* ExportRootScope) const
	{
		return TTypedTagStaticImpl2<FFaerieInventoryUserTag>::ExportTextItem(*this, ValueStr, PortFlags);
	}

protected:
	FFaerieInventoryUserTag(const FGameplayTag& Tag) { TagName = Tag.GetTagName(); }
	static const TCHAR* GetRootTagStr() { return TEXT("Fae.Inventory.Public"); }
	friend class TTypedTagStaticImpl2<FFaerieInventoryUserTag>;
};

template<> struct TNameOf<FFaerieInventoryUserTag> { FORCEINLINE static TCHAR const* GetName() { return TEXT("FFaerieInventoryUserTag"); } };

template<>
struct TStructOpsTypeTraits<FFaerieInventoryUserTag> : public TStructOpsTypeTraitsBase2<FFaerieInventoryUserTag>
{
	enum
	{
		WithExportTextItem = true,
		WithImportTextItem = true
	};
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

	UPROPERTY(EditAnywhere, Category = "InventoryEntryUserdata")
	FEntryKey Key;

	UPROPERTY(EditAnywhere, Category = "InventoryEntryUserdata", meta = (Categories = "Fae.Inventory.Public"))
	FGameplayTagContainer Tags;

	friend bool operator==(const FInventoryEntryUserdata& Lhs, const FInventoryEntryUserdata& Rhs)
	{
		return Lhs.Key == Rhs.Key
			   && Lhs.Tags == Rhs.Tags;
	}

	friend bool operator!=(const FInventoryEntryUserdata& Lhs, const FInventoryEntryUserdata& Rhs)
	{
		return !(Lhs == Rhs);
	}
};

USTRUCT()
struct FStorageUserdata
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<UFaerieItemContainerBase> Container;

	UPROPERTY()
	TArray<FInventoryEntryUserdata> Userdata;
};

/*
 * An extension added to player inventories that stored addition userdata about items, such as favorites.
 */
UCLASS()
class FAERIEINVENTORYCONTENT_API UInventoryUserdataExtension : public UInventoryExtensionBase
{
	GENERATED_BODY()

public:
	UInventoryUserdataExtension();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	//~ UInventoryExtensionBase
	virtual void DeinitializeExtension(const UFaerieItemContainerBase* Container) override;
	virtual void PreRemoval(const UFaerieItemContainerBase* Container, const FEntryKey Key, const int32 Removal) override;
	//~ UInventoryExtensionBase

private:
		  FStorageUserdata* FindUserdataForContainer(const UFaerieItemContainerBase* Container);
	const FStorageUserdata* FindUserdataForContainer(const UFaerieItemContainerBase* Container) const;

public:
	UFUNCTION(BlueprintCallable, Category = "Faerie|UserdataExtension")
	bool DoesStackHaveTag(UFaerieItemContainerBase* Container, FEntryKey Key, FFaerieInventoryUserTag Tag) const;

	UFUNCTION(BlueprintCallable, Category = "Faerie|UserdataExtension")
	bool CanSetStackTag(UFaerieItemContainerBase* Container, FEntryKey Key, FFaerieInventoryUserTag Tag, const bool StateToSetTo) const;

	bool MarkStackWithTag(UFaerieItemContainerBase* Container, FEntryKey Key, FFaerieInventoryUserTag Tag);

	bool ClearTagFromStack(UFaerieItemContainerBase* Container, FEntryKey Key, FFaerieInventoryUserTag Tag);

private:
	UPROPERTY(Replicated, SaveGame)
	TArray<FStorageUserdata> PerStorageUserdata;
};