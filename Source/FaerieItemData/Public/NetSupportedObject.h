// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "UObject/Object.h"
#include "NetSupportedObject.generated.h"

// @todo This is a weird place to store this class...

/**
 * A simple replicated UObject.
 */
UCLASS(Abstract)
class FAERIEITEMDATA_API UNetSupportedObject : public UObject
{
	GENERATED_BODY()

public:
	// Enable full networking
	virtual bool IsSupportedForNetworking() const override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};

/**
 * A simple replicated UObject. Must be owned by an actor, or it'll complain.
 */
UCLASS(Abstract)
class FAERIEITEMDATA_API UActorSubobjectBase : public UObject
{
	GENERATED_BODY()

public:
	//~ UObject
	virtual UWorld* GetWorld() const override;

	virtual bool IsSupportedForNetworking() const override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual int32 GetFunctionCallspace(UFunction* Function, FFrame* Stack) override;
	virtual bool CallRemoteFunction(UFunction* Function, void* Parms, struct FOutParmRec* OutParms, FFrame* Stack) override;

	virtual void PostInitProperties() override;
	//~ UObject

	AActor* GetOwner() const;
	AActor* GetOwnerChecked() const;
};