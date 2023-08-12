// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "GenerationActionConfig.h"
#include "Squirrel.h"

UGenerationActionConfig::UGenerationActionConfig()
{
	Squirrel = CreateDefaultSubobject<USquirrel>("Squirrel");
}