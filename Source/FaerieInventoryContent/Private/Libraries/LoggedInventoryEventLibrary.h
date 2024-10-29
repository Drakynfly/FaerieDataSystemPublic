// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "LoggedInventoryEventLibrary.generated.h"

struct FEntryKey;
struct FFaerieInventoryTag;
struct FFaerieItemKeyBase;
struct FFaerieItemStackView;
struct FLoggedInventoryEvent;

UCLASS()
class ULoggedInventoryEventLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "LoggedInventoryEventLibrary", meta = (NativeBreakFunc))
	static void BreakLoggedInventoryEvent(const FLoggedInventoryEvent& LoggedEvent, FFaerieInventoryTag& Type, bool& Success,
										  FDateTime& Timestamp, FEntryKey& EntryTouched, TArray<FFaerieItemKeyBase>& OtherKeysTouched,
										  FFaerieItemStackView& Stack, FString& ErrorMessage);
};