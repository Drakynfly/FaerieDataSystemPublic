// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "Blueprint/UserWidget.h"
#include "BenchInteractionWidgetBase.generated.h"

/**
 *
 */
UCLASS(Abstract)
class FAERIEINVENTORYCONTENT_API UBenchInteractionWidgetBase : public UUserWidget
{
	GENERATED_BODY()

protected:
	// The behavior object of the bench.
	UPROPERTY(BlueprintReadOnly, meta = (ExposeOnSpawn), Category = "Bench Interaction Widget")
	TWeakObjectPtr<class UBenchBehaviorBase> Behavior;
};