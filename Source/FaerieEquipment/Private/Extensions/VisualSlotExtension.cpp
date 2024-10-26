// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "Extensions/VisualSlotExtension.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(VisualSlotExtension)

void UVisualSlotExtension::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams Params;
	Params.bIsPushBased = true;
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, Socket, Params);
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, ComponentTag, Params);
}

void UVisualSlotExtension::SetSocket(const FName InSocket)
{
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, Socket, this);
	Socket = InSocket;
}

void UVisualSlotExtension::SetComponentTag(const FName InComponentTag)
{
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, ComponentTag, this);
	ComponentTag = InComponentTag;
}