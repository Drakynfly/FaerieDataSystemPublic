// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "GameplayTagContainer.h"
#include "TypedGameplayTags.h"
#include "InventoryDataStructs.h"

namespace Faerie::Inventory
{
	namespace Tags
	{
		FAERIEINVENTORY_API UE_DECLARE_GAMEPLAY_TAG_TYPED_EXTERN(FFaerieInventoryTag, Addition)
		FAERIEINVENTORY_API UE_DECLARE_GAMEPLAY_TAG_TYPED_EXTERN(FFaerieInventoryTag, RemovalBase)
		FAERIEINVENTORY_API UE_DECLARE_GAMEPLAY_TAG_TYPED_EXTERN(FFaerieInventoryTag, RemovalDeletion)
		FAERIEINVENTORY_API UE_DECLARE_GAMEPLAY_TAG_TYPED_EXTERN(FFaerieInventoryTag, RemovalMoving)

		FAERIEINVENTORY_API const TSet<FFaerieInventoryTag>& RemovalTagsAllowedByDefault();
	}

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
			return CreateFailureEvent_Internal(Tags::Addition, Message);
		}

		const FGuid& GetEventID() const { return EventID; }
		const FDateTime& GetTimestamp() const { return Timestamp; }

		friend bool operator==(const FEventLog& Lhs, const FEventLog& Rhs)
		{
			return Lhs.EventID == Rhs.EventID;
		}

		friend bool operator!=(const FEventLog& Lhs, const FEventLog& Rhs)
		{
			return !(Lhs == Rhs);
		}

	public:
		// Either the Addition tag or some kind of Removal.
		FFaerieInventoryTag Type;

		// Did this event succeed?
		bool Success = false;

		// The entry that this event pertained to.
		FEntryKey EntryTouched;

		// All stacks that were modified by this event.
		TArray<FStackKey> StackKeys;

		// The number of item copies added or removed.
		int32 Amount = 0;

		// The item from this entry.
		TWeakObjectPtr<const UFaerieItem> Item;

		// Message, in case of a failure event.
		FString ErrorMessage;

		friend FArchive& operator<<(FArchive& Ar, FEventLog& Val)
		{
			return Ar << Val.Type
					  << Val.Success
					  << Val.EntryTouched
					  << Val.StackKeys
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