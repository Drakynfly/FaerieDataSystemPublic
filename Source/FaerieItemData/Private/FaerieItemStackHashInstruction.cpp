// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "FaerieItemStackHashInstruction.h"

int32 UFaerieItemStackHashInstruction::ChildHash(const UFaerieItemStackHashInstruction* Child, const FFaerieItemStackView StackView)
{
	if (ensure(IsValid(Child)))
	{
		return Child->Hash(StackView);
	}
	return 0;
}