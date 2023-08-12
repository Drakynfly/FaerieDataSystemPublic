// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "Widgets/FaerieCardBase.h"

void UFaerieCardBase::NativeConstruct()
{
	Super::NativeConstruct();

	if (RefreshOnConstruct)
	{
		Refresh();
	}
}

void UFaerieCardBase::SetItemData(const UFaerieItemDataProxyBase* InItemData, const bool bRefresh)
{
	ItemData = InItemData;
	if (bRefresh)
	{
		Refresh();
	}
}

void UFaerieCardBase::Refresh()
{
	OnCardRefreshed.Broadcast();
	BP_Refresh();
}
