// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SpatialDragVisual.generated.h"

/**
 * 
 */
UCLASS()
class FAERIEINVENTORYCONTENT_API USpatialDragVisual : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<TObjectPtr<UWidget>> WidgetRefs;

	UFUNCTION(BlueprintImplementableEvent)
	void InitializeVisual();
};
