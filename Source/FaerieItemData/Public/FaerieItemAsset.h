// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "FaerieItemSource.h"

#include "FaerieItemAsset.generated.h"

class UFaerieItem;
class UFaerieItemToken;
class UFaerieItemTemplate;

/**
 * A basic item definition. Used to generate generic items with no procedural data.
 */
UCLASS(Const)
class FAERIEITEMDATA_API UFaerieItemAsset : public UObject, public IFaerieItemSource
{
	GENERATED_BODY()

public:
	virtual void PreSave(FObjectPreSaveContext SaveContext) override;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif

	//~ IFaerieItemSource
	virtual FFaerieAssetInfo GetSourceInfo() const override;
	virtual UFaerieItem* CreateItemInstance(UObject* Outer) const override;
	//~ IFaerieItemSource

#if WITH_EDITOR
	// Gets a const ptr to the archetype item this asset generates.
	// ONLY AVAILABLE IN EDITOR!
	const UFaerieItem* GetEditorItemView() const { return Item; }
#endif

protected:
	UPROPERTY(VisibleAnywhere, DuplicateTransient, Category = "ItemAsset")
	TObjectPtr<UFaerieItem> Item;

#if WITH_EDITORONLY_DATA
	// Tokens used to build the Item. Only exist in the editor, as the item is compiled by PreSave.
	UPROPERTY(EditInstanceOnly, Instanced, Category = "ItemAsset")
	TArray<TObjectPtr<UFaerieItemToken>> Tokens;

	// Item template, used to verify that the generated item follows an expected pattern.
	UPROPERTY(EditInstanceOnly, Category = "ItemAsset")
	TObjectPtr<UFaerieItemTemplate> Template;
#endif
};