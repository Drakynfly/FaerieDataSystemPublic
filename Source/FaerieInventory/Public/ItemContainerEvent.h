﻿// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "GameplayTagContainer.h"
#include "InventoryDataStructs.h"

struct FAERIEINVENTORY_API FFaerieItemStorageEvents : public FGameplayTagNativeAdder
{
	FORCEINLINE static const FFaerieItemStorageEvents& Get() { return FaerieInventoryEvents; }

	// Tag for events where items are added to the inventory.
	FFaerieInventoryTag Addition;

	// Tag for events where items are removed from the inventory.
	FFaerieInventoryTag RemovalBaseTag;

	FFaerieInventoryTag Removal_Deletion;
	FFaerieInventoryTag Removal_Moving;

protected:
	virtual void AddTags() override
	{
		Addition = FFaerieInventoryTag::AddNativeTag(TEXT("Addition"),
				"Inventory item data added event");

		RemovalBaseTag = FFaerieInventoryTag::AddNativeTag(TEXT("Removal"),
				"Inventory item data removed event");

		Removal_Deletion = FFaerieInventoryTag::AddNativeTag(TEXT("Removal.Deletion"),
						"Remove an item by deleting it entirely");

		Removal_Moving = FFaerieInventoryTag::AddNativeTag(TEXT("Removal.Moving"),
						"Remove an item for the purpose of moving it elsewhere");

		RemovalTagsAllowedByDefault.Emplace(Removal_Deletion);
		RemovalTagsAllowedByDefault.Emplace(Removal_Moving);
	}

private:
	// Private static object for the global tags. Use the Get() function to access externally.
	static FFaerieItemStorageEvents FaerieInventoryEvents;

public:
	static TSet<FFaerieInventoryTag> RemovalTagsAllowedByDefault;
};

namespace Faerie::Inventory
{
	// Logs that record data about additions to and removals from an item container.
	class FAERIEINVENTORY_API FEventLog
	{
	public:
		FEventLog()
		  : EventID(FGuid::NewGuid()),
			Timestamp(FDateTime::UtcNow()) {}

	private:
		static FEventLog CreateFailureEvent_Internal(const FFaerieInventoryTag Type, const FString& Message)
		{
			FEventLog NewErrorEvent;
			NewErrorEvent.Type = Type;
			NewErrorEvent.Success = false;
			NewErrorEvent.ErrorMessage = Message;
			return NewErrorEvent;
		}

	public:
		static FEventLog AdditionFailed(const FString& Message)
		{
			return CreateFailureEvent_Internal(FFaerieItemStorageEvents::Get().Addition, Message);
		}

		const FDateTime& GetTimestamp() const { return Timestamp; }
		const FGuid& GetEventID() const { return EventID; }

		friend bool operator==(const FEventLog& Lhs, const FEventLog& Rhs)
		{
			return Lhs.EventID == Rhs.EventID;
		}

		friend bool operator!=(const FEventLog& Lhs, const FEventLog& Rhs)
		{
			return !(Lhs == Rhs);
		}

	public:
		FFaerieInventoryTag Type;
		bool Success = false;

		FEntryKey EntryTouched;
		TArray<FFaerieItemKeyBase> OtherKeysTouched;
		int32 Amount = 0;
		TWeakObjectPtr<const UFaerieItem> Item;
		FString ErrorMessage;

		friend FArchive& operator<<(FArchive& Ar, FEventLog& Val)
		{
			return Ar << Val.Type
					  << Val.Success
					  << Val.EntryTouched
					  << Val.OtherKeysTouched
					  << Val.Amount
					  << Val.Item
					  << Val.ErrorMessage
					  << Val.EventID
					  << Val.Timestamp;
		}

	private:
		FGuid EventID;
		FDateTime Timestamp;
	};
}