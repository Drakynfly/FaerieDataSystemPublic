// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "FaerieSlotTag.h"
#include "FaerieSlotTagLibrary.generated.h"

UCLASS()
class UFaerieSlotTagLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Returns true if the values are equal (A == B) */
	UFUNCTION(BlueprintPure, Category = "GameplayTags", meta = (DisplayName = "Equal (FaerieSlotTag)", CompactNodeTitle = "==", BlueprintThreadSafe))
	static bool EqualEqual_FaerieSlotTag(const FFaerieSlotTag A, const FFaerieSlotTag B) { return A == B; }

	/** Returns true if the values are not equal (A != B) */
	UFUNCTION(BlueprintPure, Category = "GameplayTags", meta = (DisplayName = "Not Equal (FaerieSlotTag)", CompactNodeTitle = "!=", BlueprintThreadSafe))
	static bool NotEqual_FaerieSlotTag(const FFaerieSlotTag A, const FFaerieSlotTag B)  { return A != B; }
};