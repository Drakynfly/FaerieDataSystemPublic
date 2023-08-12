// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"

#include "FaerieCardGenerateAsync.generated.h"

class UFaerieItemDataProxyBase;
class UFaerieCardBase;

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
	UFaerieCardGenerateAsync(const FObjectInitializer& ObjectInitializer);

	// Logs out of the online service
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext="WorldContextObject"), Category = "Online")
	static UFaerieCardGenerateAsync* GenerateFaerieCardAsync(TScriptInterface<IFaerieCardGeneratorInterface> Generator, UFaerieItemDataProxyBase* Proxy, EFaerieCardGeneratorType Type);

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
	TScriptInterface<IFaerieCardGeneratorInterface> Generator;

	UPROPERTY();
	TObjectPtr<UFaerieItemDataProxyBase> Proxy;

	UPROPERTY()
	EFaerieCardGeneratorType Type;
};
