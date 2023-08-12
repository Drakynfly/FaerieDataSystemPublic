// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "Extensions/InventoryUserdataExtension.h"

#include "FaerieItemStorage.h"
#include "Net/UnrealNetwork.h"

FFaerieInventoryUserTags FFaerieInventoryUserTags::FaerieInventoryUserTags;

UInventoryUserdataExtension::UInventoryUserdataExtension()
{

}

void UInventoryUserdataExtension::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams SharedParams;
	SharedParams.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, PerStorageUserdata, SharedParams);
}

void UInventoryUserdataExtension::DeinitializeExtension(const UFaerieItemContainerBase* Container)
{
	PerStorageUserdata.RemoveAll([Container](const FStorageUserdata& Userdata)
		{
			return Userdata.Container == Container;
		});
}

void UInventoryUserdataExtension::PreRemoval(const UFaerieItemContainerBase* Container, const FEntryKey Key, const int32 Removal)
{
	if (auto&& Userdata = FindUserdataForContainer(Container))
	{
		if (Userdata->Userdata.RemoveAllSwap([Key](const FInventoryEntryUserdata& Datum)
		{
			return Datum.Key == Key;
		}) != 0)
		{
			MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, PerStorageUserdata, this);
		}
	}
}

FStorageUserdata* UInventoryUserdataExtension::FindUserdataForContainer(const UFaerieItemContainerBase* Container)
{
	return PerStorageUserdata.FindByPredicate([Container](const FStorageUserdata& Userdata)
		{
			return Userdata.Container == Container;
		});
}

const FStorageUserdata* UInventoryUserdataExtension::FindUserdataForContainer(const UFaerieItemContainerBase* Container) const
{
	return PerStorageUserdata.FindByPredicate([Container](const FStorageUserdata& Userdata)
		{
			return Userdata.Container == Container;
		});
}

bool UInventoryUserdataExtension::DoesStackHaveTag(UFaerieItemContainerBase* Container, FEntryKey Key, const FFaerieInventoryUserTag Tag) const
{
	if (auto&& Userdata = FindUserdataForContainer(Container))
	{
		if (auto&& Userdatum = Userdata->Userdata.FindByPredicate([Key](const FInventoryEntryUserdata& Datum)
		{
			return Datum.Key == Key;
		}))
		{
			return Userdatum->Tags.HasTag(Tag);
		}
	}

	return false;
}

bool UInventoryUserdataExtension::CanSetStackTag(UFaerieItemContainerBase* Container, const FEntryKey Key, const FFaerieInventoryUserTag Tag,
                                                  const bool StateToSetTo) const
{
	return DoesStackHaveTag(Container, Key, Tag) != StateToSetTo;
}

bool UInventoryUserdataExtension::MarkStackWithTag(UFaerieItemContainerBase* Container, const FEntryKey Key, const FFaerieInventoryUserTag Tag)
{
	if (!Tag.IsValid())
	{
		return false;
	}

	if (!CanSetStackTag(Container, Key, Tag, true))
	{
		return false;
	}

	if (!Container->IsValidKey(Key))
	{
		return false;
	}

	FStorageUserdata* Userdata = FindUserdataForContainer(Container);
	if (!Userdata)
	{
		Userdata = &PerStorageUserdata.AddDefaulted_GetRef();
		Userdata->Container = Container;
	}

	if (auto&& Userdatum = Userdata->Userdata.FindByPredicate([Key](const FInventoryEntryUserdata& Datum)
		{
			return Datum.Key == Key;
		}))
	{
		Userdatum->Tags.AddTag(Tag);
	}
	else
	{
		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, PerStorageUserdata, this);
		Userdata->Userdata.Add({Key, Tag.GetSingleTagContainer()});
	}

	return true;
}

bool UInventoryUserdataExtension::ClearTagFromStack(UFaerieItemContainerBase* Container, const FEntryKey Key, const FFaerieInventoryUserTag Tag)
{
	if (!Tag.IsValid())
	{
		return false;
	}

	if (!CanSetStackTag(Container, Key, Tag, false))
	{
		return false;
	}

	if (auto&& Userdata = FindUserdataForContainer(Container))
	{
		if (auto&& Userdatum = Userdata->Userdata.FindByPredicate([Key](const FInventoryEntryUserdata& Datum)
    		{
    			return Datum.Key == Key;
    		}))
    	{
    		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, PerStorageUserdata, this);
    		return Userdatum->Tags.RemoveTag(Tag);
    	}
	}

	return false;
}