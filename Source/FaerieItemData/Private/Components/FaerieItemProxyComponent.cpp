// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "Components/FaerieItemProxyComponent.h"

UFaerieItemProxyComponent::UFaerieItemProxyComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UFaerieItemProxyComponent::SetItemProxy(const FFaerieItemProxy Proxy)
{
	ItemProxy = Proxy;
	OnItemSet.Broadcast(this, ItemProxy);
}

void UFaerieItemProxyComponent::ClearItemProxy()
{
	ItemProxy = nullptr;
	OnItemClear.Broadcast(this);
}