// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "Subsystems/LocalPlayerSubsystem.h"
#include "FaerieCardGeneratorInterface.h"

#include "FaerieCardSubsystem.generated.h"

/**
 * A per-player subsystem providing a default FaerieCardGenerator. Use this to call GenerateItemCard / GenerateItemCardAsync
 */
UCLASS()
class FAERIEITEMCARD_API UFaerieCardSubsystem : public ULocalPlayerSubsystem, public IFaerieCardGeneratorInterface
{
	GENERATED_BODY()

public:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	virtual UFaerieCardGenerator* GetGenerator() const override;

private:
	UPROPERTY()
	TObjectPtr<UFaerieCardGenerator> Generator;
};