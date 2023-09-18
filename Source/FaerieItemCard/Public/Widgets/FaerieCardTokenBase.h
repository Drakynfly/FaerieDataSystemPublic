// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "Blueprint/UserWidget.h"
#include "FaerieCardTokenBase.generated.h"

class UFaerieItemToken;

USTRUCT()
struct FItemCardSparseClassData
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = "Item Card Class Data", meta = (AllowAbstract = true))
	TSubclassOf<UFaerieItemToken> TokenClass;
};

/**
 *
 */
UCLASS(SparseClassDataTypes = "ItemCardSparseClassData")
class FAERIEITEMCARD_API UFaerieCardTokenBase : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;
	virtual void NativeDestruct() override;

protected:
	void OnCardRefreshed();

	UFUNCTION(BlueprintImplementableEvent, Category = "Faerie|ItemCardToken", meta = (DisplayName = "Refresh"))
	void BP_Refresh();

protected:
	UPROPERTY(BlueprintReadOnly, Category = "ItemCardToken")
	TObjectPtr<const UFaerieItemToken> ItemToken;
};