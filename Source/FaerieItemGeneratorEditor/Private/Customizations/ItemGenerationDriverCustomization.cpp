// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "ItemGenerationDriverCustomization.h"

#include "DetailLayoutBuilder.h"
#include "ItemGeneratorConfig.h"
#include "ItemSourcePool.h"

TSharedRef<IDetailCustomization> FItemGenerationDriverCustomization::MakeInstance()
{
	return MakeShareable(new FItemGenerationDriverCustomization());
}

void FItemGenerationDriverCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	auto&& PoolProp = DetailBuilder.GetProperty(UItemGenerationDriver::GetMemberName_Pool());

	UObject* PoolObj;
	PoolProp->GetValue(PoolObj);
	if (IsValid(PoolObj))
	{
		auto&& DropListProp = PoolProp->GetChildHandle(UItemSourcePool::GetMemberName_DropList());
		DetailBuilder.AddPropertyToCategory(DropListProp);
	}
}