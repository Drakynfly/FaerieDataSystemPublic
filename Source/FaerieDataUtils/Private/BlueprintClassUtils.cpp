// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "BlueprintClassUtils.h"

UObject* UBlueprintClassUtils::GetTypedOuter(UObject* Object, const TSubclassOf<UObject> Class)
{
	if (!IsValid(Object)) return nullptr;
	return Object->GetTypedOuter(Class);
}