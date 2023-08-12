// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "FaerieItemMutator.h"
#include "FaerieItemDataFilter.h"
#include "FaerieItemTemplate.h"

bool UFaerieItemMutator::CanApply(const UFaerieItemDataProxyBase* Container) const
{
	if (!IsValid(ApplicationFilter) || IsValid(ApplicationFilter->GetPattern())) return false;
	return ApplicationFilter->GetPattern()->Exec(Container);
}

bool UFaerieItemMutator::TryApply(UFaerieItemDataProxyBase* Entry)
{
	if (CanApply(Entry))
	{
		return Apply(Entry);
	}
	return false;
}

void UFaerieItemMutator::GetRequiredAssets_Implementation(TArray<TSoftObjectPtr<UObject>>& RequiredAssets) const {}