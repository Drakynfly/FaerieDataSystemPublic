// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

class FFaerieInventoryModule final : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};