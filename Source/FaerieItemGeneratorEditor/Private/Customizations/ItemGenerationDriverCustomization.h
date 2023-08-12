// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "IDetailCustomization.h"

class FItemGenerationDriverCustomization : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();

	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
};
