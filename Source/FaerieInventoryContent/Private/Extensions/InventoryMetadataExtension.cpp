// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "Extensions/InventoryMetadataExtension.h"
#include "Extensions/InventoryEjectionHandlerExtension.h"
#include "FaerieItemStorage.h"

FFaerieInventoryMetaTags FFaerieInventoryMetaTags::FaerieInventoryMetaTags;

UInventoryMetadataExtension::UInventoryMetadataExtension()
{
}

void UInventoryMetadataExtension::DeinitializeExtension(const UFaerieItemContainerBase* Container)
{
	PerStorageMetadata.Remove(Container);
}

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

	if (auto&& StorageData = PerStorageMetadata.Find(Container))
	{
		if (auto&& Metadatum = StorageData->Metadata.Find(Key))
		{
			FGameplayTagContainer ThisEventTags = RemovalDenyingTags;

			// Check for a tag that might deny this reason
			if (auto&& DenialTag = OtherDenialTags.Find(Reason))
			{
				ThisEventTags.AddTag(*DenialTag);
			}

			if (Metadatum->Tags.HasAny(ThisEventTags))
			{
				return EEventExtensionResponse::Disallowed;
			}
		}
	}

	return EEventExtensionResponse::Allowed;
}

void UInventoryMetadataExtension::PreRemoval(const UFaerieItemContainerBase* Container, const FEntryKey Key, const int32 Removal)
{
	if (auto&& StorageData = PerStorageMetadata.Find(Container))
	{
		(*StorageData).Metadata.Remove(Key);
	}
}

bool UInventoryMetadataExtension::DoesEntryHaveTag(UFaerieItemContainerBase* Container, const FEntryKey Key, const FFaerieInventoryMetaTag Tag) const
{
	if (auto&& StorageData = PerStorageMetadata.Find(Container))
	{
		if (auto&& Metadatum = StorageData->Metadata.Find(Key))
		{
			return Metadatum->Tags.HasTag(Tag);
		}
	}
	return false;
}

bool UInventoryMetadataExtension::CanSetEntryTag(UFaerieItemContainerBase* Container, const FEntryKey Key,
												 const FFaerieInventoryMetaTag Tag, const bool StateToSetTo) const
{
	return DoesEntryHaveTag(Container, Key, Tag) != StateToSetTo;
}

bool UInventoryMetadataExtension::MarkStackWithTag(UFaerieItemContainerBase* Container, const FEntryKey Key, const FFaerieInventoryMetaTag Tag)
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

	PerStorageMetadata.FindOrAdd(Container).Metadata.FindOrAdd(Key).Tags.AddTag(Tag);
	return true;
}

void UInventoryMetadataExtension::TrySetTags(UFaerieItemContainerBase* Container, const FEntryKey Key, const FGameplayTagContainer Tags)
{
	// @todo implement
	unimplemented();
}

bool UInventoryMetadataExtension::ClearTagFromStack(UFaerieItemContainerBase* Container, const FEntryKey Key, const FFaerieInventoryMetaTag Tag)
{
	if (!Tag.IsValid())
	{
		return false;
	}

	if (!CanSetEntryTag(Container, Key, Tag, false))
	{
		return false;
	}

	if (auto&& StorageData = PerStorageMetadata.Find(Container))
	{
		if (auto* Metadatum = StorageData->Metadata.Find(Key))
		{
			return Metadatum->Tags.RemoveTag(Tag);
		}
	}

	return false;
}
