// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "FaerieItemSource.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FaerieItemSource)

UFaerieItem* IFaerieItemSource::CreateItemInstance(UObject* Outer) const
{
	unimplemented();
	return nullptr;
}

UFaerieItem* IFaerieItemSource::CreateItemInstance(const UItemInstancingContext* Context) const
{
	return CreateItemInstance(Context->Outer);
}