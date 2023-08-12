// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "FaerieItemOwnerInterface.h"
#include "FaerieItemStack.h"
#include "FaerieItemDataProxy.generated.h"

UCLASS(Abstract, BlueprintType)
class FAERIEITEMDATA_API UFaerieItemDataProxyBase : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Faerie|ItemProxy")
	virtual const UFaerieItem* GetItemObject() const PURE_VIRTUAL(UFaerieItemDataProxyBase::GetItemData, return nullptr; )

	UFUNCTION(BlueprintCallable, Category = "Faerie|ItemProxy")
	virtual int32 GetCopies() const PURE_VIRTUAL(UFaerieItemDataProxyBase::GetCopies, return -1; )

	UFUNCTION(BlueprintCallable, Category = "Faerie|ItemProxy")
	virtual TScriptInterface<IFaerieItemOwnerInterface> GetOwner() PURE_VIRTUAL(UFaerieItemDataProxyBase::GetOwner, return nullptr; )
};

UCLASS(meta = (DontUseGenericSpawnObject = "true"))
class FAERIEITEMDATA_API UFaerieItemDataStackLiteral : public UFaerieItemDataProxyBase, public IFaerieItemOwnerInterface
{
	GENERATED_BODY()

public:
	//~ UFaerieItemDataProxyBase
	virtual const UFaerieItem* GetItemObject() const override;
	virtual int32 GetCopies() const override;
	virtual TScriptInterface<IFaerieItemOwnerInterface> GetOwner() override { return this; }
	//~ UFaerieItemDataProxyBase

	//~ IFaerieItemOwnerInterface
	virtual FFaerieItemStack Release(FFaerieItemStackView Stack) override;
	virtual bool Possess(FFaerieItemStack Stack) override;
	//~ IFaerieItemOwnerInterface

public:
	void SetValue(UFaerieItem* Item);

	UFUNCTION(BlueprintCallable, Category = "Faerie|ItemDataStackLiteral")
	void SetValue(FFaerieItemStack InStack);

	UFUNCTION(BlueprintCallable, Category = "Faerie|ItemDataStackLiteral")
	static UFaerieItemDataStackLiteral* CreateItemDataStackLiteral(FFaerieItemStack InStack);

protected:
	UPROPERTY(BlueprintReadOnly, Category = "ItemDataStackLiteral")
	FFaerieItemStack ItemStack;
};

/**
 * A proxy for a stack that something else owns.
 */
UCLASS(meta = (DontUseGenericSpawnObject = "true"))
class FAERIEITEMDATA_API UFaerieItemDataStackViewLiteral : public UFaerieItemDataProxyBase
{
	GENERATED_BODY()

public:
	//~ UFaerieItemDataProxyBase
	virtual const UFaerieItem* GetItemObject() const override;
	virtual int32 GetCopies() const override;
	virtual TScriptInterface<IFaerieItemOwnerInterface> GetOwner() override;
	//~ UFaerieItemDataProxyBase

	UFUNCTION(BlueprintCallable, Category = "Faerie|ItemDataStackViewLiteral")
	void SetValue(FFaerieItemStackView InStack, TScriptInterface<IFaerieItemOwnerInterface> InOwner);

	UFUNCTION(BlueprintCallable, Category = "Faerie|ItemDataStackViewLiteral")
	static UFaerieItemDataStackViewLiteral* CreateItemDataStackLiteral(FFaerieItemStackView InStack, TScriptInterface<IFaerieItemOwnerInterface> InOwner);

protected:
	UPROPERTY(BlueprintReadOnly, Category = "ItemDataStackViewLiteral")
	FFaerieItemStackView Stack;

	UPROPERTY(BlueprintReadOnly, Category = "ItemDataStackViewLiteral")
	TScriptInterface<IFaerieItemOwnerInterface> Owner;
};