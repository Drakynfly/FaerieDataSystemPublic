// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "FaerieItemDataFilter.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FaerieItemDataFilter)

bool UFaerieItemDataFilter::ExecWithLog(const FFaerieItemStackView View,
										Faerie::ItemData::FFilterLogger& Logger) const
{
	const bool Result = Exec(View);
	if (!Result)
	{
		static const FTextFormat ErrorFormat = NSLOCTEXT("FaerieItemDataFilter", "GenericFilterError", "Filter '{0}' failed. Implement ExecWithLog for more details.");
		FFormatOrderedArguments Args;
		Args.Add(GetClass()->GetDisplayNameText());
		Logger.Errors.Add(FText::Format(ErrorFormat, Args));
	}

	return Result;
}