// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "Blueprint/UserWidget.h"
#include "FaerieCardBase.generated.h"

class UFaerieItemDataProxyBase;

DECLARE_MULTICAST_DELEGATE(FOnCardRefreshed)

/**
 *
 */
UCLASS(Abstract)
class FAERIEITEMCARD_API UFaerieCardBase : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	void SetItemData(const UFaerieItemDataProxyBase* InItemData, bool bRefresh);

	TObjectPtr<const UFaerieItemDataProxyBase> GetItemData() const { return ItemData; }

	FOnCardRefreshed& GetOnCardRefreshed() { return OnCardRefreshed; }

	UFUNCTION(BlueprintCallable, Category = "Faerie|ItemCard")
	void Refresh();

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "Faerie|ItemCard", meta = (DisplayName = "Refresh"))
	void BP_Refresh();

protected:
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<const UFaerieItemDataProxyBase> ItemData;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	bool RefreshOnConstruct = true;

	FOnCardRefreshed OnCardRefreshed;
};
