// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "Extensions/InventoryContentFilterExtension.h"
#include "FaerieItemDataFilter.h"

EEventExtensionResponse UInventoryContentFilterExtension::AllowsAddition(const UFaerieItemContainerBase* Container,
                                                                         const FFaerieItemStackView Stack)
{
	if (ensure(IsValid(Filter)))
	{
		if (Filter->Exec(Stack))
		{
			return EEventExtensionResponse::Allowed;
		}

		return EEventExtensionResponse::Disallowed;
	}

	return EEventExtensionResponse::NoExplicitResponse;
}