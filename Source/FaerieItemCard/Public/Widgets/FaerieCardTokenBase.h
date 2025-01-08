﻿// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "Blueprint/UserWidget.h"
#include "FaerieCardTokenBase.generated.h"

class UFaerieItem;
class UFaerieCardBase;
class UFaerieItemToken;

USTRUCT()
struct FItemCardSparseClassData
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = "ItemCardSparseClassData", meta = (AllowAbstract = true))
	TSubclassOf<UFaerieItemToken> TokenClass;
};

/**
 *
 */
UCLASS(Abstract, SparseClassDataTypes = "ItemCardSparseClassData")
class FAERIEITEMCARD_API UFaerieCardTokenBase : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;
	virtual void NativeDestruct() override;

protected:
	void OnCardRefreshed();

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Faerie|ItemCardToken")
	const UFaerieItem* GetItem() const;

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Faerie|ItemCardToken")
	const UFaerieItemToken* GetItemToken() const;

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Faerie|ItemCardToken", meta = (ExpandBoolAsExecs = "ReturnValue"))
	bool GetItemTokenChecked(UFaerieItemToken*& Token) const;

	UFUNCTION(BlueprintImplementableEvent, Category = "Faerie|ItemCardToken", meta = (DisplayName = "Refresh"))
	void BP_Refresh();

	UFUNCTION(BlueprintCallable, Category = "Faerie|ItemCardToken")
	UFaerieCardBase* GetOwningCard() const;

protected:
	UE_DEPRECATED(5.4, "Use GetItemToken/GetItemTokenChecked instead.")
	UPROPERTY(BlueprintReadOnly, Category = "ItemCardToken")
	TObjectPtr<const UFaerieItemToken> ItemToken;
};