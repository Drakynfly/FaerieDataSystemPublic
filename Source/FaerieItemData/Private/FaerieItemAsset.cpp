// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "FaerieItemAsset.h"

#include "FaerieItem.h"
#include "FaerieItemToken.h"
#include "FaerieItemTemplate.h"
#include "Misc/DataValidation.h"
#include "Tokens/FaerieInfoToken.h"
#include "UObject/ObjectSaveContext.h"

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

EDataValidationResult UFaerieItemAsset::IsDataValid(FDataValidationContext& Context)
{
	bool HasError = false;

	if (!Item)
	{
		Context.AddError(LOCTEXT("InvalidItemObject", "Item is invalid! Please try making sure Tokens are correctly configured and resave this asset."));
		HasError = true;
	}

	if (!IsValid(Template))
	{
		Context.AddWarning(LOCTEXT("InvalidTemplateObject", "Template is invalid! Unable to check Item for pattern-correctness."));
	}

	if (Item && IsValid(Template))
	{
		UFaerieItemDataStackLiteral* Literal = NewObject<UFaerieItemDataStackLiteral>();
		Literal->SetValue(Item);

		if (!Template->TryMatch(Literal))
		{
			Context.AddError(LOCTEXT("PatternMatchFailed", "Item failed to match the pattern of its Template!"));
			HasError = true;
		}
	}

	if (HasError)
	{
		return EDataValidationResult::Invalid;
	}

	return Super::IsDataValid(Context);
}

FFaerieAssetInfo UFaerieItemAsset::GetSourceInfo() const
{
	if (auto&& InfoToken = Item->GetToken<UFaerieInfoToken>())
	{
		return InfoToken->GetInfoObject();
	}
	return FFaerieAssetInfo();
}

#undef LOCTEXT_NAMESPACE

#endif

UFaerieItem* UFaerieItemAsset::CreateItemInstance(UObject* Outer) const
{
	UFaerieItem* NewInstance;

	if (Item->IsDataMutable())
	{
		// Make a copy of the static item stored in this asset if we might need to modify the data
		NewInstance = Item->CreateDuplicate();

		// @todo this is super weird. why are we doing this here???
		AActor* Actor = NewInstance->GetTypedOuter<AActor>();
		if (IsValid(Actor))
		{
#if !NO_LOGGING
			if (!Actor->IsUsingRegisteredSubObjectList())
			{
				UE_LOG(LogTemp, Warning,
					TEXT("Actor outer '%s' for item instance '%s' does not replicate using SubObject list. Item will not be replicated normally!"),
					*Actor->GetName(), *NewInstance->GetName())
			}
#endif

			Actor->AddReplicatedSubObject(NewInstance);
		}
	}
	else
	{
		// If the item is not mutable, we can just reference the single copy of it.
		NewInstance = Item;
	}

	return NewInstance;
}