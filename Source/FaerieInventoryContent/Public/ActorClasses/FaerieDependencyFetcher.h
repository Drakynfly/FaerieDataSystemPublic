// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "Components/ActorComponent.h"
#include "FaerieDependencyFetcher.generated.h"

/*
 * Base class for dependency injection of components. A child of this class can be found or passed into inventory widgets,
 * where other components can be fetched using this without knowing exactly where they live.
 */
UCLASS(Blueprintable, ClassGroup = ("Faerie"), meta = (BlueprintSpawnableComponent))
class FAERIEINVENTORYCONTENT_API UFaerieDependencyFetcher : public UActorComponent
{
	GENERATED_BODY()

public:
	UFaerieDependencyFetcher();

	UFUNCTION(BlueprintCallable, BlueprintPure = false, BlueprintNativeEvent, Category = "Faerie|DependencyFetcher",
		meta = (ExpandBoolAsExecs = "ReturnValue", DeterminesOutputType = "Class", DynamicOutputParam = "Component"))
	bool GetDependency(TSubclassOf<UActorComponent> Class, UActorComponent*& Component) const;
};