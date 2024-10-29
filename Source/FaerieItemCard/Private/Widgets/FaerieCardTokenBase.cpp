// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "Widgets/FaerieCardTokenBase.h"

#include "FaerieItem.h"
#include "FaerieItemDataProxy.h"
#include "FaerieItemToken.h"
#include "Widgets/FaerieCardBase.h"

void UFaerieCardTokenBase::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (auto&& OuterCard = GetTypedOuter<UFaerieCardBase>())
	{
		OuterCard->GetOnCardRefreshed().AddUObject(this, &ThisClass::OnCardRefreshed);
	}
}

void UFaerieCardTokenBase::NativeDestruct()
{
	if (auto&& OuterCard = GetTypedOuter<UFaerieCardBase>())
	{
		OuterCard->GetOnCardRefreshed().RemoveAll(this);
	}

	Super::NativeDestruct();
}

void UFaerieCardTokenBase::OnCardRefreshed()
{
	if (!IsValid(GetTokenClass()))
	{
		return;
	}

	PRAGMA_DISABLE_DEPRECATION_WARNINGS
	ItemToken = nullptr;

	if (auto&& OuterCard = GetTypedOuter<UFaerieCardBase>())
	{
		if (auto&& ItemData = OuterCard->GetItemData();
			ItemData.IsValid())
		{
			if (auto&& Object = ItemData->GetItemObject())
			{
				ItemToken = Object->GetToken(GetTokenClass());
			}
		}
	}
	PRAGMA_ENABLE_DEPRECATION_WARNINGS

	BP_Refresh();
}

UFaerieCardBase* UFaerieCardTokenBase::GetOwningCard() const
{
	return GetTypedOuter<UFaerieCardBase>();
}