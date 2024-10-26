// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "FaerieItemToken.h"
#include "FaerieItemStorageToken.generated.h"

class UFaerieItemContainerBase;
class UFaerieItemStorage;

/**
 * Base class for tokens that add child item containers to items
 */
UCLASS(Abstract)
class FAERIEINVENTORY_API UFaerieItemContainerToken : public UFaerieItemToken
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool IsMutable() const override;

	// Get all container objects from ContainerTokens.
	UFUNCTION(BlueprintCallable, Category = "Faerie|ItemContainerToken")
	static TSet<UFaerieItemContainerBase*> GetAllContainersInItem(const UFaerieItem* Item);

	UFaerieItemContainerBase* GetItemContainer() { return ItemContainer; }
	const UFaerieItemContainerBase* GetItemContainer() const { return ItemContainer; }

protected:
	UPROPERTY(Replicated)
	TObjectPtr<UFaerieItemContainerBase> ItemContainer;
};

class UItemContainerExtensionGroup;

/**
 * This token adds an Item Storage object used to store items nested in another.
 */
UCLASS(DisplayName = "Token - Item Storage")
class FAERIEINVENTORY_API UFaerieItemStorageToken : public UFaerieItemContainerToken
{
	GENERATED_BODY()

public:
	UFaerieItemStorageToken();

	UFUNCTION(BlueprintCallable, Category = "Faerie|ItemStorage")
	UFaerieItemStorage* GetItemStorage() const;

protected:
	UPROPERTY(EditInstanceOnly, Instanced, NoClear, Category = "ItemStorage")
	TObjectPtr<UItemContainerExtensionGroup> Extensions;
};