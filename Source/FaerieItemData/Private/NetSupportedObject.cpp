// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.


#include "NetSupportedObject.h"

UWorld* UNetSupportedObject::GetWorld() const
{
	if (const AActor* Owner = GetOwner())
	{
		return Owner->GetWorld();
	}

	return nullptr;
}

bool UNetSupportedObject::IsSupportedForNetworking() const
{
	return IsValid(GetOwner());
}

void UNetSupportedObject::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Add any Blueprint properties
	if (const UBlueprintGeneratedClass* BPClass = Cast<UBlueprintGeneratedClass>(GetClass()))
	{
		BPClass->GetLifetimeBlueprintReplicationList(OutLifetimeProps);
	}
}

int32 UNetSupportedObject::GetFunctionCallspace(UFunction* Function, FFrame* Stack)
{
	check(GetOuter() != nullptr);
	return GetOuter()->GetFunctionCallspace(Function, Stack);
}

bool UNetSupportedObject::CallRemoteFunction(UFunction* Function, void* Parms, FOutParmRec* OutParms, FFrame* Stack)
{
	check(!HasAnyFlags(RF_ClassDefaultObject));

	// Call "Remote" (aka, RPC) functions through the actors NetDriver

	AActor* Owner = GetOwner();
	if (IsValid(Owner))
	{
		if (UNetDriver* NetDriver = Owner->GetNetDriver())
		{
			NetDriver->ProcessRemoteFunction(Owner, Function, Parms, OutParms, Stack, this);
			return true;
		}
	}

	return false;
}

AActor* UNetSupportedObject::GetOwner() const
{
	AActor* Owner = GetTypedOuter<AActor>();
	return Owner;
}

AActor* UNetSupportedObject::GetOwnerChecked() const
{
	AActor* Owner = GetOwner();
	check(Owner);
	return Owner;
}