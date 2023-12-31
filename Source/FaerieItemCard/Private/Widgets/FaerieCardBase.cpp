﻿// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "Widgets/FaerieCardBase.h"

void UFaerieCardBase::NativeConstruct()
{
	Super::NativeConstruct();

	if (RefreshOnConstruct)
	{
		Refresh();
	}
}

void UFaerieCardBase::SetItemData(const FFaerieItemProxy InItemProxy, const bool bRefresh)
{
	ItemProxy = InItemProxy;
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