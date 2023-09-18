// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

class FFaerieInventoryContentModule final : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};