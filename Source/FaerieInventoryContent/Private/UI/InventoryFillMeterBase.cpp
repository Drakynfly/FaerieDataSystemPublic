// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "UI/InventoryFillMeterBase.h"
#include "Extensions/InventoryCapacityExtension.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(InventoryFillMeterBase)

void UInventoryFillMeterBase::InitWithExtension(UInventoryCapacityExtension* Extension)
{
	if (Extension && Extension != CapacityModule)
	{
		CapacityModule = Extension;
		OnInit();
	}
}

void UInventoryFillMeterBase::InitWithoutExtension()
{
	OnInitEmpty();
}

void UInventoryFillMeterBase::Reset()
{
	OnReset();
	CapacityModule = nullptr;
}