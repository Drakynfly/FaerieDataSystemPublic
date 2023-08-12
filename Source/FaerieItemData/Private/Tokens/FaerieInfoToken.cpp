// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "Tokens/FaerieInfoToken.h"
#include "FaerieInfoObject.h"
#include "Net/UnrealNetwork.h"

#define LOCTEXT_NAMESPACE "FaerieInfoToken"

void UFaerieInfoToken::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ThisClass, Info, COND_InitialOnly);
}

bool UFaerieInfoToken::IsMutable() const
{
	// Info tokens are inherantly immutable, and they must be since they are used to identify items.
	// This doesn't mean that an item *cannot* be renamed, just that if it is, it's considered a seperate item.
	return false;
}

bool UFaerieInfoToken::CompareWithImpl(const UFaerieItemToken* FaerieItemToken) const
{
	if (auto&& Other = Cast<UFaerieInfoToken>(FaerieItemToken))
	{
		if (!Info.ObjectName.IdenticalTo(Other->Info.ObjectName))
		{
			return false;
		}
	}

	return true;
}

const FFaerieAssetInfo& UFaerieInfoToken::GetInfoObject() const
{
	return Info;
}

FText UFaerieInfoToken::GetItemName() const
{
	return Info.ObjectName;
}

FText UFaerieInfoToken::GetShortDescription() const
{
	return Info.ShortDescription;
}

FText UFaerieInfoToken::GetLongDescription() const
{
	return Info.LongDescription;
}

TSoftObjectPtr<UTexture2D> UFaerieInfoToken::GetIcon() const
{
	return Info.Icon;
}

#undef LOCTEXT_NAMESPACE