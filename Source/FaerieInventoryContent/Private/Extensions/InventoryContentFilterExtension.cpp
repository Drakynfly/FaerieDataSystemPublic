// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "Extensions/InventoryContentFilterExtension.h"
#include "FaerieItemDataFilter.h"
#include "FaerieItemDataProxy.h"

UInventoryContentFilterExtension::UInventoryContentFilterExtension()
{
	ExecutionContainer = CreateDefaultSubobject<UFaerieItemDataStackViewLiteral>("ExecutionContainer");
}

EEventExtensionResponse UInventoryContentFilterExtension::AllowsAddition(const UFaerieItemContainerBase* Container,
                                                                         const FFaerieItemStackView Stack)
{
	if (ensure(IsValid(Filter)))
	{
		ExecutionContainer->SetValue(Stack, nullptr);

		if (Filter->Exec(ExecutionContainer))
		{
			return EEventExtensionResponse::Allowed;
		}

		return EEventExtensionResponse::Disallowed;
	}

	return EEventExtensionResponse::NoExplicitResponse;
}