// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "UObject/Object.h"
#include "FaerieItemDataTypes.h"
#include "FaerieItemDataFilter.generated.h"

class UFaerieItemDataProxyBase;

/**
 *
 */
UCLASS(Abstract, Const, EditInlineNew, DefaultToInstanced, CollapseCategories)
class FAERIEITEMDATA_API UFaerieItemDataFilter : public UObject
{
	GENERATED_BODY()

public:
#if WITH_EDITOR
	// This function allows the owning object to know if this filter will allow mutable or immutable assets through, or
	// if it doesn't know. The default is unknown, and specific children must override one way or the other.
	// This function is only called in the editor and saved to a variable when needed at runtime.
	virtual EItemDataMutabilityStatus GetMutabilityStatus() const { return EItemDataMutabilityStatus::Unknown; }
#endif

	UFUNCTION(BlueprintCallable, Category = "Faerie|ItemDataFilter")
	virtual bool Exec(const UFaerieItemDataProxyBase* Proxy) const PURE_VIRTUAL(UFaerieItemDataFilter::Exec, return false; )
};

USTRUCT(BlueprintType)
struct FAERIEITEMDATA_API FInlineFaerieItemDataFilter
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Instanced, Category = "Inline Inventory Filter Filter")
	TObjectPtr<UFaerieItemDataFilter> Filter = nullptr;
};
