// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "GameplayTagContainer.h"
#include "ItemContainerExtensionBase.h"
#include "InventoryReplicatedDataExtensionBase.h"
#include "InventoryMetadataExtension.generated.h"

/**
 * The key used to tag entries with custom client data.
 */
USTRUCT(BlueprintType, meta = (Categories = "Fae.Inventory.Meta"))
struct FAERIEINVENTORYCONTENT_API FFaerieInventoryMetaTag : public FFaerieInventoryTag
{
	GENERATED_BODY()
	END_TAG_DECL2(FFaerieInventoryMetaTag, TEXT("Fae.Inventory.Meta"))
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

/**
 * An extension for programmatic control over entry key permissions.
 */
UCLASS()
class FAERIEINVENTORYCONTENT_API UInventoryMetadataExtension : public UInventoryReplicatedDataExtensionBase
{
	GENERATED_BODY()

protected:
	//~ UItemContainerExtensionBase
	virtual EEventExtensionResponse AllowsRemoval(const UFaerieItemContainerBase* Container, const FEntryKey Key, const FFaerieInventoryTag Reason) const override;
	//~ UItemContainerExtensionBase

	//~ UInventoryReplicatedDataExtensionBase
	virtual UScriptStruct* GetDataScriptStruct() const override;
	//~ UInventoryReplicatedDataExtensionBase

public:
	bool DoesEntryHaveTag(const UFaerieItemContainerBase* Container, const FEntryKey Key, FFaerieInventoryMetaTag Tag) const;

	bool CanSetEntryTag(const UFaerieItemContainerBase* Container, const FEntryKey Key, const FFaerieInventoryMetaTag Tag, const bool StateToSetTo) const;

	bool MarkStackWithTag(const UFaerieItemContainerBase* Container, const FEntryKey Key, FFaerieInventoryMetaTag Tag);
	void TrySetTags(const UFaerieItemContainerBase* Container, const FEntryKey Key, FGameplayTagContainer Tags);

	bool ClearTagFromStack(const UFaerieItemContainerBase* Container, const FEntryKey Key, FFaerieInventoryMetaTag Tag);
};