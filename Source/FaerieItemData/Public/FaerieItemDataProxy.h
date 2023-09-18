// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "FaerieItemOwnerInterface.h"
#include "FaerieItemStack.h"
#include "FaerieItemDataProxy.generated.h"


/**
 * A simple implementation of ItemDataProxy that *does* own its item, denoted by the 'Literal' suffix.
 * It therefor implements IFaerieItemOwnerInterface as well, to allow usage of Release/Possess.
 */
UCLASS(meta = (DontUseGenericSpawnObject = "true"))
class FAERIEITEMDATA_API UFaerieItemDataStackLiteral : public UObject, public IFaerieItemDataProxy, public IFaerieItemOwnerInterface
{
	GENERATED_BODY()

public:
	//~ UFaerieItemDataProxy
	virtual const UFaerieItem* GetItemObject() const override;
	virtual int32 GetCopies() const override;
	virtual bool CanMutate() const override { return true; }
	virtual TScriptInterface<IFaerieItemOwnerInterface> GetOwner() const override { return const_cast<ThisClass*>(this); }
	//~ UFaerieItemDataProxy

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