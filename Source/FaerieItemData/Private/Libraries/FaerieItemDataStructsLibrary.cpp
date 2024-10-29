// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "FaerieItemDataStructsLibrary.h"
#include "FaerieItemStack.h"

int32 UFaerieItemDataStructsLibrary::UnlimitedStack()
{
	return Faerie::ItemData::UnlimitedStack;
}

bool UFaerieItemDataStructsLibrary::IsUnlimited(const int32 Stack)
{
	return Stack == Faerie::ItemData::UnlimitedStack;
}