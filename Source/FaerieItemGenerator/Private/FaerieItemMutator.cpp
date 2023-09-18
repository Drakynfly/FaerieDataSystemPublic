// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "FaerieItemMutator.h"
#include "FaerieItemDataFilter.h"
#include "FaerieItemTemplate.h"

bool UFaerieItemMutator::CanApply(const FFaerieItemProxy Proxy) const
{
	return Proxy->CanMutate() &&
		IsValid(ApplicationFilter) &&
		ApplicationFilter->TryMatch(Proxy);
}

bool UFaerieItemMutator::TryApply(const FFaerieItemStack Stack)
{
	if (IsValid(ApplicationFilter) &&
		!ApplicationFilter->TryMatch(Stack))
	{
		return false;
	}

	return Apply(Stack);
}

void UFaerieItemMutator::GetRequiredAssets_Implementation(TArray<TSoftObjectPtr<UObject>>& RequiredAssets) const {}