// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "BlueprintClassUtils.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BlueprintClassUtils)

UObject* UBlueprintClassUtils::GetTypedOuter(UObject* Object, const TSubclassOf<UObject> Class)
{
	if (!IsValid(Object)) return nullptr;
	return Object->GetTypedOuter(Class);
}