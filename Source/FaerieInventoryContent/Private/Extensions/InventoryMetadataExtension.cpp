// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "Extensions/InventoryMetadataExtension.h"
#include "Extensions/InventoryEjectionHandlerExtension.h"
#include "FaerieItemStorage.h"

FFaerieInventoryMetaTags FFaerieInventoryMetaTags::FaerieInventoryMetaTags;

EEventExtensionResponse UInventoryMetadataExtension::AllowsRemoval(const UFaerieItemContainerBase* Container,
	const FEntryKey Key, const FFaerieInventoryTag Reason) const
{
	// Tags that always deny removal.
	static FGameplayTagContainer RemovalDenyingTags = FGameplayTagContainer::CreateFromArray(
		TArray<FGameplayTag>{
			FFaerieInventoryMetaTags::Get().CannotRemove
		});

	// Tags that deny a specific reason
	static TMap<FFaerieInventoryTag, FFaerieInventoryTag> OtherDenialTags = {
		{ FFaerieItemStorageEvents::Get().Removal_Deletion, FFaerieInventoryMetaTags::Get().CannotDelete },
		{ FFaerieItemStorageEvents::Get().Removal_Moving, FFaerieInventoryMetaTags::Get().CannotMove },
		{ FFaerieEjectionEvent::Get().Removal_Ejection, FFaerieInventoryMetaTags::Get().CannotEject }
	};

	FGameplayTagContainer ThisEventTags = RemovalDenyingTags;

	// Check for a tag that might deny this reason
	if (auto&& DenialTag = OtherDenialTags.Find(Reason))
	{
		ThisEventTags.AddTag(*DenialTag);
	}

	if (const FConstStructView DataView = GetDataForEntry(Container, Key);
		DataView.IsValid())
	{
		if (DataView.Get<const FInventoryEntryMetadata>().Tags.HasAny(ThisEventTags))
		{
			return EEventExtensionResponse::Disallowed;
		}
	}

	return EEventExtensionResponse::Allowed;
}

UScriptStruct* UInventoryMetadataExtension::GetDataScriptStruct() const
{
	return FInventoryEntryMetadata::StaticStruct();
}

bool UInventoryMetadataExtension::DoesEntryHaveTag(const UFaerieItemContainerBase* Container, const FEntryKey Key, const FFaerieInventoryMetaTag Tag) const
{
	const FConstStructView DataView = GetDataForEntry(Container, Key);
	if (!DataView.IsValid())
	{
		return false;
	}

	return DataView.Get<const FInventoryEntryMetadata>().Tags.HasTag(Tag);
}

bool UInventoryMetadataExtension::CanSetEntryTag(const UFaerieItemContainerBase* Container, const FEntryKey Key,
												 const FFaerieInventoryMetaTag Tag, const bool StateToSetTo) const
{
	return DoesEntryHaveTag(Container, Key, Tag) != StateToSetTo;
}

bool UInventoryMetadataExtension::MarkStackWithTag(const UFaerieItemContainerBase* Container, const FEntryKey Key, const FFaerieInventoryMetaTag Tag)
{
	if (!Tag.IsValid())
	{
		return false;
	}

	if (!CanSetEntryTag(Container, Key, Tag, true))
	{
		return false;
	}

	if (!Container->IsValidKey(Key))
	{
		return false;
	}

	return EditDataForEntry(Container, Key,
		[Tag](FInstancedStruct& Data)
		{
			Data.GetMutable<FInventoryEntryMetadata>().Tags.AddTag(Tag);
		});
}

void UInventoryMetadataExtension::TrySetTags(const UFaerieItemContainerBase* Container, const FEntryKey Key, const FGameplayTagContainer Tags)
{
	// @todo implement
	unimplemented();
}

bool UInventoryMetadataExtension::ClearTagFromStack(const UFaerieItemContainerBase* Container, const FEntryKey Key, const FFaerieInventoryMetaTag Tag)
{
	if (!Tag.IsValid())
	{
		return false;
	}

	if (!CanSetEntryTag(Container, Key, Tag, false))
	{
		return false;
	}

	return EditDataForEntry(Container, Key,
		[Tag](FInstancedStruct& Data)
		{
			Data.GetMutable<FInventoryEntryMetadata>().Tags.RemoveTag(Tag);
		});
}