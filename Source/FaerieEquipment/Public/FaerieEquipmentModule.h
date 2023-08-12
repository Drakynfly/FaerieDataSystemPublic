// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

class FFaerieEquipmentModule final : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};