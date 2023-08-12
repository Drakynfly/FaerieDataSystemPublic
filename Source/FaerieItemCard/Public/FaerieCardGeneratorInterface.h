// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "UObject/Interface.h"
#include "FaerieCardGeneratorInterface.generated.h"

class UFaerieCardGenerator;

// This class does not need to be modified.
UINTERFACE()
class UFaerieCardGeneratorInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 *
 */
class FAERIEITEMCARD_API IFaerieCardGeneratorInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual UFaerieCardGenerator* GetGenerator() const PURE_VIRTUAL(IFaerieCardGeneratorInterface::GetGenerator, return nullptr; )
};
