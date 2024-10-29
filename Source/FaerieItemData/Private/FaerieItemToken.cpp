// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "FaerieItemToken.h"
#include "FaerieItem.h"

bool UFaerieItemToken::IsMutable() const
{
	return false;
}

bool UFaerieItemToken::CompareWithImpl(const UFaerieItemToken* Other) const
{
	return true;
}

bool UFaerieItemToken::IsOuterItemMutable() const
{
	auto&& OuterItem = GetOuterItem();
	return IsValid(OuterItem) ? OuterItem->IsDataMutable() : false;
}

void UFaerieItemToken::NotifyOuterOfChange()
{
	auto&& Item = GetOuterItem();
	if (!Item)
	{
		return;
	}

	Item->OnTokenEdited(this);
}

UFaerieItem* UFaerieItemToken::GetOuterItem() const
{
	return GetTypedOuter<UFaerieItem>();
}

bool UFaerieItemToken::CompareWith(const UFaerieItemToken* Other) const
{
	// Auto-success if we are the same pointer
	if (this == Other) return true;

	// Auto-failure for not matching classes
	if (!IsValid(Other) || !Other->IsA(GetClass())) return false;

	// Run child-implemented data comparison
	return CompareWithImpl(Other);
}

void UFaerieItemToken::EditToken(const TFunctionRef<bool(UFaerieItemToken*)>& EditFunc)
{
	if (EditFunc(this))
	{
		NotifyOuterOfChange();
	}
}

UFaerieItem* UFaerieItemToken::BP_GetFaerieItem() const
{
	return GetOuterItem();
}

void UFaerieItemToken::BP_EditToken(const FBlueprintTokenEdit& Edit)
{
	if (!ensure(Edit.IsBound()))
	{
		return;
	}

	EditToken(
		[Edit](UFaerieItemToken* Token)
		{
			return Edit.Execute(Token);
		});
}