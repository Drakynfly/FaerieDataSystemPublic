// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "UObject/Interface.h"
#include "FaerieCardGeneratorInterface.generated.h"

class UFaerieCardGenerator;

UINTERFACE()
class UFaerieCardGeneratorInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface to add to classes that provide a FaerieCardGenerator. By default, a UFaerieCardSubsystem will be instanced
 * for each player, unless disabled in project settings.
 */
class FAERIEITEMCARD_API IFaerieCardGeneratorInterface
{
	GENERATED_BODY()

public:
	virtual UFaerieCardGenerator* GetGenerator() const PURE_VIRTUAL(IFaerieCardGeneratorInterface::GetGenerator, return nullptr; )
};