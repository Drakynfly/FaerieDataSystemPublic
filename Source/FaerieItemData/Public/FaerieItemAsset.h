// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "FaerieItemSource.h"

#include "FaerieItemAsset.generated.h"

class UFaerieItem;
class UFaerieItemToken;
class UFaerieItemTemplate;

/**
 * A basic item definition. Can be used to generate generic items with no procedural data.
 */
UCLASS(Const)
class FAERIEITEMDATA_API UFaerieItemAsset : public UObject, public IFaerieItemSource
{
	GENERATED_BODY()

public:
	virtual void PreSave(FObjectPreSaveContext SaveContext) override;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) override;
#endif

	//~ IFaerieItemSource
	virtual FFaerieAssetInfo GetSourceInfo() const override;
	virtual UFaerieItem* CreateItemInstance(UObject* Outer) const override;
	//~ IFaerieItemSource

protected:
	UPROPERTY(VisibleAnywhere, DuplicateTransient)
	TObjectPtr<UFaerieItem> Item;

#if WITH_EDITORONLY_DATA
	// Tokens used to build the Item. Only exist in the editor, as the item is compiled by PreSave.
	UPROPERTY(EditInstanceOnly, Instanced)
	TArray<TObjectPtr<UFaerieItemToken>> Tokens;

	UPROPERTY(EditInstanceOnly)
	TObjectPtr<UFaerieItemTemplate> Template;
#endif
};