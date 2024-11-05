// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "FaerieItemAsset.h"

#include "FaerieItem.h"
#include "FaerieItemTemplate.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#include "FaerieItemDataProxy.h"
#include "Tokens/FaerieInfoToken.h"
#include "UObject/ObjectSaveContext.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FaerieItemAsset)

void UFaerieItemAsset::PreSave(FObjectPreSaveContext SaveContext)
{
#if WITH_EDITOR
	if (!Item)
	{
		Item = NewObject<UFaerieItem>(this);
	}

	Item->Tokens.Empty();
	for (auto&& Token : Tokens)
	{
		Item->Tokens.Add(DuplicateObject(Token, Item));
	}
#endif

	Super::PreSave(SaveContext);
}

#if WITH_EDITOR

#define LOCTEXT_NAMESPACE "ValidateFaerieItemAsset"

EDataValidationResult UFaerieItemAsset::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = CombineDataValidationResults(Super::IsDataValid(Context), EDataValidationResult::Valid);

	if (!IsValid(Item))
	{
		Context.AddError(LOCTEXT("InvalidItemObject", "Item is invalid! Please try making sure Tokens are correctly configured and resave this asset."));
		Result = EDataValidationResult::Invalid;
	}
	else
	{
		Result = CombineDataValidationResults(Result, Item->IsDataValid(Context));
	}

	if (!IsValid(Template))
	{
		Context.AddWarning(LOCTEXT("InvalidTemplateObject", "Template is invalid! Unable to check Item for pattern-correctness."));
	}

	for (auto&& Token : Tokens)
	{
		Result = CombineDataValidationResults(Result, Token->IsDataValid(Context));
	}

	if (IsValid(Item) && IsValid(Template))
	{
		if (TArray<FText> TemplateMatchErrors;
			!Template->TryMatchWithDescriptions({Item, 1}, TemplateMatchErrors))
		{
			Context.AddError(LOCTEXT("PatternMatchFailed", "Item failed to match the pattern of its Template!"));

			for (auto&& TemplateMatchError : TemplateMatchErrors)
			{
				Context.AddError(TemplateMatchError);
			}
		}
	}

	return Result;
}

#undef LOCTEXT_NAMESPACE

#endif

FFaerieAssetInfo UFaerieItemAsset::GetSourceInfo() const
{
	if (auto&& InfoToken = Item->GetToken<UFaerieInfoToken>())
	{
		return InfoToken->GetAssetInfo();
	}
	return FFaerieAssetInfo();
}

UFaerieItem* UFaerieItemAsset::CreateItemInstance(UObject* Outer) const
{
	UFaerieItem* NewInstance;

	if (Item->IsDataMutable())
	{
		// Make a copy of the static item stored in this asset if we might need to modify the data
		NewInstance = Item->CreateDuplicate();
	}
	else
	{
		// If the item is not mutable, we can just reference the single copy of it.
		NewInstance = Item;
	}

	return NewInstance;
}