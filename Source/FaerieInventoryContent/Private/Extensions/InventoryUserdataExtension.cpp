﻿// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "Extensions/InventoryUserdataExtension.h"

#include "FaerieItemStorage.h"

FFaerieInventoryUserTags FFaerieInventoryUserTags::FaerieInventoryUserTags;

UScriptStruct* UInventoryUserdataExtension::GetDataScriptStruct() const
{
	return FInventoryEntryUserdata::StaticStruct();
}

bool UInventoryUserdataExtension::DoesStackHaveTag(UFaerieItemContainerBase* Container, const FEntryKey Key, const FFaerieInventoryUserTag Tag) const
{
	const FConstStructView DataView = GetDataForEntry(Container, Key);
	if (!DataView.IsValid())
	{
		return false;
	}

	return DataView.Get<const FInventoryEntryUserdata>().Tags.HasTag(Tag);
}

bool UInventoryUserdataExtension::CanSetStackTag(UFaerieItemContainerBase* Container, const FEntryKey Key, const FFaerieInventoryUserTag Tag,
                                                  const bool StateToSetTo) const
{
	return DoesStackHaveTag(Container, Key, Tag) != StateToSetTo;
}

bool UInventoryUserdataExtension::MarkStackWithTag(UFaerieItemContainerBase* Container, const FEntryKey Key, const FFaerieInventoryUserTag Tag)
{
	if (!Tag.IsValid())
	{
		return false;
	}

	if (!CanSetStackTag(Container, Key, Tag, true))
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
			Data.GetMutable<FInventoryEntryUserdata>().Tags.AddTag(Tag);
		});
}

bool UInventoryUserdataExtension::ClearTagFromStack(UFaerieItemContainerBase* Container, const FEntryKey Key, const FFaerieInventoryUserTag Tag)
{
	if (!Tag.IsValid())
	{
		return false;
	}

	if (!CanSetStackTag(Container, Key, Tag, false))
	{
		return false;
	}

	return EditDataForEntry(Container, Key,
		[Tag](FInstancedStruct& Data)
		{
			Data.GetMutable<FInventoryEntryUserdata>().Tags.RemoveTag(Tag);
		});
}