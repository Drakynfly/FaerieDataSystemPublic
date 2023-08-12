// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "CapacityTokenDetailsCustomization.h"

#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "Tokens/FaerieCapacityToken.h"

TSharedRef<IDetailCustomization> FCapacityTokenDetailCustomization::MakeInstance()
{
	return MakeShareable(new FCapacityTokenDetailCustomization());
}

void FCapacityTokenDetailCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	auto&& CapacityProp = DetailBuilder.GetProperty("Capacity");

	auto&& TokenCat = DetailBuilder.EditCategory("Token");
	TokenCat.AddProperty(CapacityProp->GetChildHandle(GET_MEMBER_NAME_CHECKED(FItemCapacity, Weight)));
}
