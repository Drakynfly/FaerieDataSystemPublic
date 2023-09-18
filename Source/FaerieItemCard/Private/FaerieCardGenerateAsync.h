// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "FaerieItemDataProxy.h"
#include "Kismet/BlueprintAsyncActionBase.h"

#include "FaerieCardGenerateAsync.generated.h"

class UFaerieCardGenerator;
class IFaerieCardGeneratorInterface;
class UFaerieCardBase;
class UCustomCardClass;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FCardGenerateFailed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCardGenerateSuccess, UFaerieCardBase*, Card);

/**
 *
 */
UCLASS()
class FAERIEITEMCARD_API UFaerieCardGenerateAsync : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Faerie|ItemCards", meta = (ExpandBoolAsExecs = "ReturnValue"))
	static bool GenerateItemCard(APlayerController* OwningPlayer, TScriptInterface<IFaerieCardGeneratorInterface> Generator, FFaerieItemProxy Proxy, TSubclassOf<UCustomCardClass> Type, UFaerieCardBase*& Widget);

	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"), Category = "Online")
	static UFaerieCardGenerateAsync* GenerateItemCardAsync(APlayerController* OwningPlayer, TScriptInterface<IFaerieCardGeneratorInterface> Generator, FFaerieItemProxy Proxy, TSubclassOf<UCustomCardClass> Type);

	// UBlueprintAsyncActionBase interface
	virtual void Activate() override;
	// End of UBlueprintAsyncActionBase interface

private:
	void OnCardGenerationFinished(bool Success, UFaerieCardBase* Widget);

protected:
	UPROPERTY(BlueprintAssignable)
	FCardGenerateSuccess OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FCardGenerateFailed OnFailure;

private:
	UPROPERTY()
	TObjectPtr<UFaerieCardGenerator> Generator;

	UPROPERTY()
	TObjectPtr<APlayerController> OwningPlayer;

	UPROPERTY();
	FFaerieItemProxy Proxy;

	UPROPERTY();
	TSubclassOf<UCustomCardClass> Class;
};