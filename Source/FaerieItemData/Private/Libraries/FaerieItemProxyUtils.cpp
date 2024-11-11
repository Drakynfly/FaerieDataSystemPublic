// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "FaerieItemProxyUtils.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FaerieItemProxyUtils)

bool UFaerieItemProxyUtils::CastProxy(const FFaerieItemProxy Proxy, UClass* Class, UObject*& ProxyObject)
{
	// @Todo the const-safety here! ProxyObject escapes into blueprint as non-const (BP doesn't understand const objects)
	ProxyObject = const_cast<UObject*>(Proxy.GetObject());
	return ProxyObject->IsA(Class);
}

FFaerieItemProxy UFaerieItemProxyUtils::ToWeakProxy(const TScriptInterface<IFaerieItemDataProxy>& ScriptProxy)
{
	return FFaerieItemProxy(ScriptProxy);
}

const UObject* UFaerieItemProxyUtils::GetProxyObject(const FFaerieItemProxy Proxy)
{
	return Proxy.GetObject();
}

bool UFaerieItemProxyUtils::IsValid(const FFaerieItemProxy Proxy)
{
	return Proxy.IsValid();
}

const UFaerieItem* UFaerieItemProxyUtils::GetItemObject(const FFaerieItemProxy Proxy)
{
	if (Proxy.IsValid())
	{
		return Proxy.GetInterface()->GetItemObject();
	}
	return nullptr;
}

int32 UFaerieItemProxyUtils::GetCopies(const FFaerieItemProxy Proxy)
{
	if (Proxy.IsValid())
	{
		return Proxy.GetInterface()->GetCopies();
	}
	return 0;
}

TScriptInterface<IFaerieItemOwnerInterface> UFaerieItemProxyUtils::GetOwner(const FFaerieItemProxy Proxy)
{
	if (Proxy.IsValid())
	{
		return Proxy.GetInterface()->GetOwner();
	}
	return nullptr;
}

FFaerieItemStackView UFaerieItemProxyUtils::ProxyToView(const FFaerieItemProxy Proxy)
{
	return FFaerieItemStackView(GetItemObject(Proxy), GetCopies(Proxy));
}