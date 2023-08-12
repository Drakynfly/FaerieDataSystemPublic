// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "FaerieItemToken.h"
#include "FaerieItemStorageToken.generated.h"

class UFaerieItemStorage;

/**
 *
 */
UCLASS()
class FAERIEINVENTORY_API UFaerieItemStorageToken : public UFaerieItemToken
{
	GENERATED_BODY()

public:
	UFaerieItemStorageToken();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool IsMutable() const override;

	UFaerieItemStorage* GetItemStorage() const { return ItemStorage; }

protected:
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "ItemStorageToken")
	TObjectPtr<UFaerieItemStorage> ItemStorage;
};