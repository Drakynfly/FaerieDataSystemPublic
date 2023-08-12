// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "GameplayTagContainer.h"
#include "InventoryExtensionBase.h"
#include "InventoryMetadataExtension.generated.h"

/**
 * The key used to tag entries with custom client data.
 */
USTRUCT(BlueprintType, meta = (Categories = "Fae.Inventory.Meta"))
struct FAERIEINVENTORYCONTENT_API FFaerieInventoryMetaTag : public FFaerieInventoryTag
{
	GENERATED_BODY()

	FFaerieInventoryMetaTag() {}
	static FFaerieInventoryMetaTag GetRootTag() { return TTypedTagStaticImpl2<FFaerieInventoryMetaTag>::StaticImpl.RootTag; }
	static FFaerieInventoryMetaTag TryConvert(const FGameplayTag FromTag) { return TTypedTagStaticImpl2<FFaerieInventoryMetaTag>::TryConvert(FromTag, false); }
	static FFaerieInventoryMetaTag ConvertChecked(const FGameplayTag FromTag) { return TTypedTagStaticImpl2<FFaerieInventoryMetaTag>::TryConvert(FromTag, true); }
	static FFaerieInventoryMetaTag AddNativeTag(const FString& TagBody, const FString& DevComment) { return TTypedTagStaticImpl2<FFaerieInventoryMetaTag>::AddNativeTag(TagBody, DevComment); }
	bool ExportTextItem(FString& ValueStr, const FFaerieInventoryMetaTag& DefaultValue, UObject* Parent, const int32 PortFlags, UObject* ExportRootScope) const
	{
		return TTypedTagStaticImpl2<FFaerieInventoryMetaTag>::ExportTextItem(*this, ValueStr, PortFlags);
	}

protected:
	FFaerieInventoryMetaTag(const FGameplayTag& Tag) { TagName = Tag.GetTagName(); }
	static const TCHAR* GetRootTagStr() { return TEXT("Fae.Inventory.Meta"); }
	friend class TTypedTagStaticImpl2<FFaerieInventoryMetaTag>;
};

template<> struct TNameOf<FFaerieInventoryMetaTag> { FORCEINLINE static TCHAR const* GetName() { return TEXT("FFaerieInventoryMetaTag"); } };

template<>
struct TStructOpsTypeTraits<FFaerieInventoryMetaTag> : public TStructOpsTypeTraitsBase2<FFaerieInventoryMetaTag>
{
	enum
	{
		WithExportTextItem = true,
		WithImportTextItem = true
	};
};

// Server-only metadata flags
struct FAERIEINVENTORYCONTENT_API FFaerieInventoryMetaTags : public FGameplayTagNativeAdder
{
	FORCEINLINE static const FFaerieInventoryMetaTags& Get() { return FaerieInventoryMetaTags; }

	FFaerieInventoryMetaTag CannotRemove;
	FFaerieInventoryMetaTag CannotDelete;
	FFaerieInventoryMetaTag CannotMove;
	FFaerieInventoryMetaTag CannotEject;
	FFaerieInventoryMetaTag CannotSplit;

protected:
	virtual void AddTags() override
	{
		CannotRemove = FFaerieInventoryMetaTag::AddNativeTag(TEXT("CannotRemove"),
						"Denies permission for the user to remove this entry. Typically used to mark required quest items.");

		CannotDelete = FFaerieInventoryMetaTag::AddNativeTag(TEXT("CannotDelete"),
						"Denies permission for the user to delete this entry. Can still be otherwise removed!");

		CannotMove = FFaerieInventoryMetaTag::AddNativeTag(TEXT("CannotMove"),
						"Denies permission for the user to move this entry.");

		CannotEject = FFaerieInventoryMetaTag::AddNativeTag(TEXT("CannotEject"),
						"Denies permission for the user to eject this entry.");

		CannotSplit = FFaerieInventoryMetaTag::AddNativeTag(TEXT("CannotSplit"),
						"Denies permission to split a stack. Typically used to mark required quest item stacks.");
	}

private:
	// Private static object for the global tags. Use the Get() function to access externally.
	static FFaerieInventoryMetaTags FaerieInventoryMetaTags;
};

USTRUCT()
struct FInventoryEntryMetadata
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "InventoryEntryUserdata", meta = (Categories = "Fae.Inventory.Meta"))
	FGameplayTagContainer Tags;
};

USTRUCT()
struct FStorageMetadata
{
	GENERATED_BODY()

	UPROPERTY()
	TMap<FEntryKey, FInventoryEntryMetadata> Metadata;
};

/**
 *
 */
UCLASS()
class FAERIEINVENTORYCONTENT_API UInventoryMetadataExtension : public UInventoryExtensionBase
{
	GENERATED_BODY()

public:
	UInventoryMetadataExtension();

protected:
	//~ UInventoryExtensionBase
	virtual void DeinitializeExtension(const UFaerieItemContainerBase* Container) override;
	virtual EEventExtensionResponse AllowsRemoval(const UFaerieItemContainerBase* Container, const FEntryKey Key, const FFaerieInventoryTag Reason) const override;
	virtual void PreRemoval(const UFaerieItemContainerBase* Container, const FEntryKey Key, const int32 Removal) override;
	//~ UInventoryExtensionBase

	bool DoesEntryHaveTag(UFaerieItemContainerBase* Container, const FEntryKey Key, FFaerieInventoryMetaTag Tag) const;

	bool CanSetEntryTag(UFaerieItemContainerBase* Container, const FEntryKey Key, const FFaerieInventoryMetaTag Tag, const bool StateToSetTo) const;

	bool MarkStackWithTag(UFaerieItemContainerBase* Container, const FEntryKey Key, FFaerieInventoryMetaTag Tag);
	void TrySetTags(UFaerieItemContainerBase* Container, const FEntryKey Key, FGameplayTagContainer Tags);

	bool ClearTagFromStack(UFaerieItemContainerBase* Container, const FEntryKey Key, FFaerieInventoryMetaTag Tag);

private:
	// Doesn't replicate or serialize, and shouldn't. For any tags that should, use UInventoryUserdataExtension instead.
	// This is used exclusively for procedural metadata, such as quest tags.
	UPROPERTY()
	TMap<TWeakObjectPtr<const UFaerieItemContainerBase>, FStorageMetadata> PerStorageMetadata;
};
