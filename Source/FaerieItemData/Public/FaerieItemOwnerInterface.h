// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "FaerieItemStack.h"
#include "UObject/Interface.h"
#include "FaerieItemOwnerInterface.generated.h"

class UFaerieItem;

UINTERFACE(NotBlueprintable, MinimalAPI)
class UFaerieItemOwnerInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 *
 */
class FAERIEITEMDATA_API IFaerieItemOwnerInterface
{
	GENERATED_BODY()

public:
	virtual FFaerieItemStack Release(FFaerieItemStackView Stack) PURE_VIRTUAL(IFaerieItemOwnerInterface::Release, return FFaerieItemStack(); )
	virtual bool Possess(FFaerieItemStack Stack) PURE_VIRTUAL(IFaerieItemOwnerInterface::Possess, return false; )
};
