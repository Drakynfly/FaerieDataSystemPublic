// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "CraftingActionConfig.h"
#include "Squirrel.h"

UCraftingActionConfig::UCraftingActionConfig()
{
	Squirrel = CreateDefaultSubobject<USquirrel>(FName{TEXTVIEW("Squirrel")});
}