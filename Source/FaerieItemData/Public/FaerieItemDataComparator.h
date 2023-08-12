// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "UObject/Object.h"
#include "FaerieItemDataComparator.generated.h"

class UFaerieItemDataProxyBase;

/**
 * Compares two item data proxies. Used to create sorting functionality.
 */
UCLASS(Abstract, Const, EditInlineNew, DefaultToInstanced, CollapseCategories)
class FAERIEITEMDATA_API UFaerieItemDataComparator : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Faerie|ItemDataComparator")
	virtual bool Exec(const UFaerieItemDataProxyBase* A, const UFaerieItemDataProxyBase* B) const PURE_VIRTUAL(UFaerieItemDataComparator::Exec, return false; )
};

USTRUCT(BlueprintType)
struct FAERIEITEMDATA_API FInlineFaerieItemDataComparator
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Instanced, Category = "Inline Inventory Filter Filter")
	TObjectPtr<UFaerieItemDataComparator> Filter = nullptr;
};