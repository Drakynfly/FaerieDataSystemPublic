// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "FaerieItemTemplate.h"
#include "FaerieItemDataFilter.h"
#include "FaerieItemDataProxy.h"

#define LOCTEXT_NAMESPACE "FaerieItemTemplate"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#if WITH_EDITOR

EDataValidationResult UFaerieItemTemplate::IsDataValid(FDataValidationContext& Context) const
{
	if (!IsValid(Pattern))
	{
		Context.AddError(NSLOCTEXT("ValidateFaerieItemTemplate", "InvalidPatternError", "Template Pattern is invalid!"));
	}

	if (Context.GetNumErrors())
	{
		return EDataValidationResult::Invalid;
	}

	return Super::IsDataValid(Context);
}

#endif

bool UFaerieItemTemplate::TryMatchWithDescriptions(const FFaerieItemStackView View, TArray<FText>& Errors) const
{
	if (!ensure(IsValid(Pattern)))
	{
		static const FTextFormat GenericErrorFormat = LOCTEXT("ExecWithErrors_InvalidPattern", "'{this}' contains Invalid Pattern!'");

		FFormatNamedArguments Args;
		Args.Add("this", FText::FromString(GetName()));

		Errors.Add(FText::Format(GenericErrorFormat, Args));
		return false;
	}

	if (Faerie::ItemData::FFilterLogger Logger;
		!Pattern->ExecWithLog(View, Logger))
	{
		if (Logger.Errors.IsEmpty())
		{
			static const FTextFormat GenericErrorFormat = LOCTEXT("ExecWithErrors_GenericErrorFmt", "'{this}' failed with unspecified reason!'");

			FFormatNamedArguments Args;
			Args.Add("this", FText::FromString(GetName()));

			Errors.Add(FText::Format(GenericErrorFormat, Args));
		}
		else
		{
			Errors = Logger.Errors;
		}

		return false;
	}

	return true;
}

bool UFaerieItemTemplate::TryMatch(const FFaerieItemStackView View) const
{
	if (ensure(IsValid(Pattern)))
	{
		return Pattern->Exec(View);
	}
	return false;
}

#undef LOCTEXT_NAMESPACE