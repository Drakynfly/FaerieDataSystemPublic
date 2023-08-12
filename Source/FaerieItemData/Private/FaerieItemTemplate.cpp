// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "FaerieItemTemplate.h"
#include "FaerieItemDataFilter.h"
#include "Misc/DataValidation.h"

#if WITH_EDITOR

EDataValidationResult UFaerieItemTemplate::IsDataValid(FDataValidationContext& Context)
{
	bool HasError = false;

	if (!Pattern)
	{
		Context.AddError(NSLOCTEXT("ValidateFaerieItemTemplate", "InvalidPatternError", "Template Pattern is invalid!"));
		HasError = true;
	}

	if (HasError)
	{
		return EDataValidationResult::Invalid;
	}

	return Super::IsDataValid(Context);
}
#endif

bool UFaerieItemTemplate::TryMatch(const UFaerieItemDataProxyBase* Proxy) const
{
	if (ensure(Pattern))
	{
		return Pattern->Exec(Proxy);
	}

	return false;
}