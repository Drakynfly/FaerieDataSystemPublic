// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "Tokens/FaerieInfoToken.h"
#include "Net/UnrealNetwork.h"

#define LOCTEXT_NAMESPACE "FaerieInfoToken"

void UFaerieInfoToken::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ThisClass, Info, COND_InitialOnly);
}

bool UFaerieInfoToken::CompareWithImpl(const UFaerieItemToken* Other) const
{
	if (auto&& AsInfo = CastChecked<UFaerieInfoToken>(Other))
	{
		if (!Info.ObjectName.IdenticalTo(AsInfo->Info.ObjectName))
		{
			return false;
		}
	}

	return true;
}

const FFaerieAssetInfo& UFaerieInfoToken::GetAssetInfo() const
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