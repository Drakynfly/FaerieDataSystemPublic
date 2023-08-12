// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "FaerieItemToken.h"
#include "CustomCardClass.generated.h"

class UFaerieCardBase;

/**
 *
 */
UCLASS(Abstract, HideDropdown)
class FAERIEITEMCARD_API UCustomCardClass : public UFaerieItemToken
{
	GENERATED_BODY()

public:
	TSoftClassPtr<UFaerieCardBase> GetCardClass() const { return CardClass; }

protected:
	UPROPERTY(EditInstanceOnly)
	TSoftClassPtr<UFaerieCardBase> CardClass;
};


UCLASS(DisplayName = "Token - Info Card Class")
class FAERIEITEMCARD_API UCustomInfoCard : public UCustomCardClass
{
	GENERATED_BODY()
};

UCLASS(DisplayName = "Token - Palette Card Class")
class FAERIEITEMCARD_API UCustomPaletteCard : public UCustomCardClass
{
	GENERATED_BODY()
};
