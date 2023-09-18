// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "FaerieItemProxy.h"
#include "FaerieItemDataProxy.h"

const UFaerieItem* FFaerieItemProxy::GetItemObject() const
{
	if (IsValid())
	{
		return Proxy->GetItemObject();
	}
	return nullptr;
}

int32 FFaerieItemProxy::GetCopies() const
{
	if (IsValid())
	{
		return Proxy->GetCopies();
	}
	return 0;
}