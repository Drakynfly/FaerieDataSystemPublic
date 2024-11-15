// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "GameplayTagContainer.h"
#include "ItemContainerExtensionBase.h"
#include "InventoryReplicatedDataExtensionBase.h"
#include "TypedGameplayTags.h"
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
namespace Faerie::Inventory::Tags
{
	FAERIEINVENTORYCONTENT_API UE_DECLARE_GAMEPLAY_TAG_TYPED_EXTERN(FFaerieInventoryMetaTag, CannotRemove)
	FAERIEINVENTORYCONTENT_API UE_DECLARE_GAMEPLAY_TAG_TYPED_EXTERN(FFaerieInventoryMetaTag, CannotDelete)
	FAERIEINVENTORYCONTENT_API UE_DECLARE_GAMEPLAY_TAG_TYPED_EXTERN(FFaerieInventoryMetaTag, CannotMove)
	FAERIEINVENTORYCONTENT_API UE_DECLARE_GAMEPLAY_TAG_TYPED_EXTERN(FFaerieInventoryMetaTag, CannotEject)
	FAERIEINVENTORYCONTENT_API UE_DECLARE_GAMEPLAY_TAG_TYPED_EXTERN(FFaerieInventoryMetaTag, CannotSplit)
}

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
	virtual EEventExtensionResponse AllowsRemoval(const UFaerieItemContainerBase* Container, FEntryKey Key, FFaerieInventoryTag Reason) const override;
	//~ UItemContainerExtensionBase

	//~ UInventoryReplicatedDataExtensionBase
	virtual UScriptStruct* GetDataScriptStruct() const override;
	virtual bool SaveRepDataArray() const override { return true; }
	//~ UInventoryReplicatedDataExtensionBase

public:
	bool DoesEntryHaveTag(const UFaerieItemContainerBase* Container, const FEntryKey Key, FFaerieInventoryMetaTag Tag) const;

	bool CanSetEntryTag(const UFaerieItemContainerBase* Container, const FEntryKey Key, const FFaerieInventoryMetaTag Tag, const bool StateToSetTo) const;

	bool MarkStackWithTag(const UFaerieItemContainerBase* Container, const FEntryKey Key, FFaerieInventoryMetaTag Tag);

	// @todo tag type-safety
	void TrySetTags(const UFaerieItemContainerBase* Container, const FEntryKey Key, const FGameplayTagContainer& Tags);

	bool ClearTagFromStack(const UFaerieItemContainerBase* Container, const FEntryKey Key, FFaerieInventoryMetaTag Tag);
};