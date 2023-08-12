// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "FaerieItemSource.h"

// Add default functionality here for any IFaerieItemSource functions that are not pure virtual.

UFaerieItem* IFaerieItemSource::CreateItemInstance(UObject* Outer) const
{
	unimplemented();
	return nullptr;
}

UFaerieItem* IFaerieItemSource::CreateItemInstance(const UItemInstancingContext* Context) const
{
	return CreateItemInstance(Context->Outer);
}