// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "FaerieItemDataProxy.h"
#include "Components/ActorComponent.h"
#include "FaerieItemProxyComponent.generated.h"

class UFaerieItemProxyComponent;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FItemSetEvent, UFaerieItemProxyComponent*, Component, FFaerieItemProxy, Proxy);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FItemClearEvent, UFaerieItemProxyComponent*, Component);


/**
 * A simple actor component that holds an item proxy struct, and offers events for the implementing actor to use.
 */
UCLASS(ClassGroup = ("Faerie"), meta = (BlueprintSpawnableComponent))
class FAERIEITEMDATA_API UFaerieItemProxyComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFaerieItemProxyComponent();

	FFaerieItemProxy GetItemProxy() const { return ItemProxy; }

	UFUNCTION(BlueprintCallable, Category = "Faerie|ItemProxyComponent")
	void SetItemProxy(FFaerieItemProxy Proxy);

	UFUNCTION(BlueprintCallable, Category = "Faerie|ItemProxyComponent")
	void ClearItemProxy();

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FItemSetEvent OnItemSet;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FItemClearEvent OnItemClear;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "ItemProxy")
	FFaerieItemProxy ItemProxy;
};