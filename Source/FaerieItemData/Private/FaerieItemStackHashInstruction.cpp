// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "FaerieItemStackHashInstruction.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FaerieItemStackHashInstruction)

int32 UFaerieItemStackHashInstruction::ChildHash(const UFaerieItemStackHashInstruction* Child, const FFaerieItemStackView StackView)
{
	if (ensure(IsValid(Child)))
	{
		return Child->Hash(StackView);
	}
	return 0;
}