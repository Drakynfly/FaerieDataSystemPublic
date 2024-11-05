// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "Libraries/LoggedInventoryEventLibrary.h"
#include "Extensions/InventoryLoggerExtension.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LoggedInventoryEventLibrary)

void ULoggedInventoryEventLibrary::BreakLoggedInventoryEvent(const FLoggedInventoryEvent& LoggedEvent, FFaerieInventoryTag& Type,
															 bool& Success, FDateTime& Timestamp, FEntryKey& EntryTouched,
															 TArray<FFaerieItemKeyBase>& OtherKeysTouched, FFaerieItemStackView& Stack, FString& ErrorMessage)
{
	Type = LoggedEvent.Event.Type;
	Success = LoggedEvent.Event.Success;
	Timestamp = LoggedEvent.Event.GetTimestamp();
	EntryTouched = LoggedEvent.Event.EntryTouched;
	OtherKeysTouched = LoggedEvent.Event.OtherKeysTouched;
	Stack.Copies = LoggedEvent.Event.Amount;
	Stack.Item = LoggedEvent.Event.Item.IsValid() ? LoggedEvent.Event.Item.Get() : nullptr;
	ErrorMessage = LoggedEvent.Event.ErrorMessage;
}