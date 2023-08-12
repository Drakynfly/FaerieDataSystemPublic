// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "FaerieItemToken.h"
#include "FaerieItem.h"

void UFaerieItemToken::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Add any Blueprint properties
	if (const UBlueprintGeneratedClass* BPClass = Cast<UBlueprintGeneratedClass>(GetClass()))
	{
		BPClass->GetLifetimeBlueprintReplicationList(OutLifetimeProps);
	}
}

bool UFaerieItemToken::IsMutable() const
{
	return false;
}

bool UFaerieItemToken::CompareWithImpl(const UFaerieItemToken* FaerieItemToken) const
{
	return true;
}

bool UFaerieItemToken::IsOuterItemMutable() const
{
	auto&& OuterItem = GetOuterItem();
	return IsValid(OuterItem) ? OuterItem->IsDataMutable() : false;
}

UFaerieItem* UFaerieItemToken::GetOuterItem() const
{
	return GetTypedOuter<UFaerieItem>();
}

bool UFaerieItemToken::CompareWith(const UFaerieItemToken* FaerieItemToken) const
{
	// Auto-success if we are the same pointer
	if (this == FaerieItemToken) return true;

	// Auto-failure for not matching classes
	if (!IsValid(FaerieItemToken) || !FaerieItemToken->IsA(GetClass())) return false;

	// Run child-implemented data comparison
	return CompareWithImpl(FaerieItemToken);
}

void UFaerieItemToken::EditToken(const TFunction<bool(UFaerieItemToken*)>& EditFunc)
{
	if (EditFunc(this))
	{
		auto&& Item = GetOuterItem();
		if (!Item)
		{
			return;
		}

		Item->OnTokenEdited(this);
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

	EditToken([Edit](UFaerieItemToken* Token){
		return Edit.Execute(Token);
	});
}