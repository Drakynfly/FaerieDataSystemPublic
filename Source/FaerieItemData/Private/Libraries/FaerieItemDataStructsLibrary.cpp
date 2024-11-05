// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "FaerieItemDataStructsLibrary.h"
#include "FaerieItemStack.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FaerieItemDataStructsLibrary)

int32 UFaerieItemDataStructsLibrary::UnlimitedStack()
{
	return Faerie::ItemData::UnlimitedStack;
}

bool UFaerieItemDataStructsLibrary::IsUnlimited(const int32 Stack)
{
	return Stack == Faerie::ItemData::UnlimitedStack;
}