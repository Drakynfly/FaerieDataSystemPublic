// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "FaerieInfoObject.h"
#include "FaerieItemToken.h"

#include "FaerieInfoToken.generated.h"

UCLASS(DisplayName = "Token - Info")
class FAERIEITEMDATA_API UFaerieInfoToken : public UFaerieItemToken
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	//~ UFaerieItemToken
	virtual bool IsMutable() const override;
	virtual bool CompareWithImpl(const UFaerieItemToken* FaerieItemToken) const override;
	//~ UFaerieItemToken

	UFUNCTION(BlueprintCallable, Category = "Faerie|InfoToken")
	const FFaerieAssetInfo& GetInfoObject() const;

	UFUNCTION(BlueprintCallable, Category = "Faerie|InfoToken")
	FText GetItemName() const;

	UFUNCTION(BlueprintCallable, Category = "Faerie|InfoToken")
	FText GetShortDescription() const;

	UFUNCTION(BlueprintCallable, Category = "Faerie|InfoToken")
	FText GetLongDescription() const;

	UFUNCTION(BlueprintCallable, Category = "Faerie|InfoToken")
	TSoftObjectPtr<UTexture2D> GetIcon() const;

protected:
	UPROPERTY(EditAnywhere, Replicated, Category = "StaticInfoToken", NoClear)
	FFaerieAssetInfo Info;
};