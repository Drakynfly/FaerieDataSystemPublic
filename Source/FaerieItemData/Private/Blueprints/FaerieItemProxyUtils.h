// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "FaerieItemDataProxy.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FaerieItemProxyUtils.generated.h"

class IFaerieItemOwnerInterface;
class UFaerieItem;

/**
 *
 */
UCLASS()
class UFaerieItemProxyUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// SOMEWHAT TEMP FIX
	UFUNCTION(BlueprintCallable, Category = "Faerie|ItemProxyUtils", meta = (DeterminesOutputType = "Class", DynamicOutputParam = "ProxyObject", ExpandBoolAsExecs = "ReturnValue"))
	static bool CastProxy(FFaerieItemProxy Proxy, UPARAM(meta = (MustImplement = "/Script/FaerieItemData.FaerieItemDataProxy")) UClass* Class, UObject*& ProxyObject);

	UFUNCTION(BlueprintPure, Category = "Faerie|ItemProxyUtils")
	static FFaerieItemProxy ToWeakProxy(TScriptInterface<IFaerieItemDataProxy> ScriptProxy);

	// Get the Object implementing the IFaerieItemDataProxy interface.
	UFUNCTION(BlueprintPure, Category = "Faerie|ItemProxyUtils")
	static const UObject* GetProxyObject(FFaerieItemProxy Proxy);

	UFUNCTION(BlueprintPure, Category = "Faerie|ItemProxyUtils")
	static bool IsValid(FFaerieItemProxy Proxy);

	// Get the Item Definition Object that is proxy represents.
	UFUNCTION(BlueprintPure, Category = "Faerie|ItemProxyUtils")
	static const UFaerieItem* GetItemObject(FFaerieItemProxy Proxy);

	// Get the number of copies this proxy may access.
	UFUNCTION(BlueprintPure, Category = "Faerie|ItemProxyUtils")
	static int32 GetCopies(FFaerieItemProxy Proxy);

	// Get the Object that owns the item this proxy represents.
	UFUNCTION(BlueprintPure, Category = "Faerie|ItemProxyUtils")
	static TScriptInterface<IFaerieItemOwnerInterface> GetOwner(FFaerieItemProxy Proxy);

	// Convert a Item Proxy into a Stack View.
	UFUNCTION(BlueprintPure, Category = "Faerie|ItemProxyUtils")
	static FFaerieItemStackView ProxyToView(FFaerieItemProxy Proxy);
};