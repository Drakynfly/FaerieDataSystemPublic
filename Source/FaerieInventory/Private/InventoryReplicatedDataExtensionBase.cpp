// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "InventoryReplicatedDataExtensionBase.h"
#include "FaerieItemContainerBase.h"
#include "StructView.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(InventoryReplicatedDataExtensionBase)

void FRepDataPerEntryBase::PreReplicatedRemove(const FRepDataFastArray& InArraySerializer)
{
	InArraySerializer.PreDataReplicatedRemove(*this);
}

void FRepDataPerEntryBase::PostReplicatedAdd(const FRepDataFastArray& InArraySerializer)
{
	InArraySerializer.PostDataReplicatedAdd(*this);
}

void FRepDataPerEntryBase::PostReplicatedChange(const FRepDataFastArray& InArraySerializer)
{
	InArraySerializer.PostDataReplicatedChange(*this);
}

void FRepDataFastArray::RemoveDataForEntry(const FEntryKey Key)
{
	if (Remove(Key,
		[this](const FRepDataPerEntryBase& Entry)
		{
			// Notify server of this removal.
			PreDataReplicatedRemove(Entry);
		}))
	{
		// Notify clients of this removal.
		MarkArrayDirty();
	}
}

void FRepDataFastArray::SetDataForEntry(const FEntryKey Key, const FInstancedStruct& Data)
{
	if (const int32 Index = IndexOf(Key);
		Index != INDEX_NONE)
	{
		FRepDataPerEntryBase& EntryData = Entries[Index];
		EntryData.Value = Data;
		MarkItemDirty(EntryData);

		// Notify server of this change.
		PostDataReplicatedChange(EntryData);
	}
	else
	{
		FRepDataPerEntryBase& NewEntry = Insert(FRepDataPerEntryBase(Key, Data));
		MarkItemDirty(NewEntry);

		// Notify server of this change.
		PostDataReplicatedAdd(NewEntry);
	}
}

void FRepDataFastArray::PreDataReplicatedRemove(const FRepDataPerEntryBase& Data) const
{
	if (OwningWrapper.IsValid())
	{
		OwningWrapper->PreContentRemoved(Data);
	}
}

void FRepDataFastArray::PostDataReplicatedAdd(const FRepDataPerEntryBase& Data) const
{
	if (OwningWrapper.IsValid())
	{
		OwningWrapper->PostContentAdded(Data);
	}
}

void FRepDataFastArray::PostDataReplicatedChange(const FRepDataPerEntryBase& Data) const
{
	if (OwningWrapper.IsValid())
	{
		OwningWrapper->PostContentChanged(Data);
	}
}

void URepDataArrayWrapper::PostInitProperties()
{
	Super::PostInitProperties();

	// Bind replication functions out into this class.
	DataArray.OwningWrapper = this;
}

void URepDataArrayWrapper::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, DataArray);
}

void URepDataArrayWrapper::PreContentRemoved(const FRepDataPerEntryBase& Data)
{
	if (Container.IsValid())
	{
		GetOuterUInventoryReplicatedDataExtensionBase()->PreEntryDataRemoved(Container.Get(), Data);
	}
}

void URepDataArrayWrapper::PostContentAdded(const FRepDataPerEntryBase& Data)
{
	if (Container.IsValid())
	{
		GetOuterUInventoryReplicatedDataExtensionBase()->PreEntryDataAdded(Container.Get(), Data);
	}
}

void URepDataArrayWrapper::PostContentChanged(const FRepDataPerEntryBase& Data)
{
	if (Container.IsValid())
	{
		GetOuterUInventoryReplicatedDataExtensionBase()->PreEntryDataChanged(Container.Get(), Data);
	}
}

void UInventoryReplicatedDataExtensionBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams SharedParams;
	SharedParams.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, PerContainerData, SharedParams);
}

FInstancedStruct UInventoryReplicatedDataExtensionBase::MakeSaveData(const UFaerieItemContainerBase* Container)
{
	if (SaveRepDataArray())
	{
		return FInstancedStruct(FindFastArrayForContainer(Container));
	}

	return FInstancedStruct();
}

void UInventoryReplicatedDataExtensionBase::LoadSaveData(const UFaerieItemContainerBase* Container,
	const FInstancedStruct& SaveData)
{
	if (const FStructView ContainerData = FindFastArrayForContainer(Container);
		ContainerData.IsValid())
	{
		FRepDataFastArray& Ref = ContainerData.Get<FRepDataFastArray>();
		Ref.Entries = SaveData.Get<FRepDataFastArray>().Entries;
		Ref.MarkArrayDirty();
	}
}

void UInventoryReplicatedDataExtensionBase::InitializeExtension(const UFaerieItemContainerBase* Container)
{
	Super::InitializeExtension(Container);

	if (ensure(IsValid(Container)))
	{
		URepDataArrayWrapper* NewWrapper = NewObject<URepDataArrayWrapper>(this);
		NewWrapper->Container = Container;

		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, PerContainerData, this);
		PerContainerData.Add(NewWrapper);
	}
}

void UInventoryReplicatedDataExtensionBase::DeinitializeExtension(const UFaerieItemContainerBase* Container)
{
	Super::DeinitializeExtension(Container);

	if (!!PerContainerData.RemoveAll(
		[Container](const TObjectPtr<URepDataArrayWrapper>& Wrapper)
		{
			return Wrapper->Container == Container;
		}))
	{
		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, PerContainerData, this);
	}
}

void UInventoryReplicatedDataExtensionBase::PreRemoval(const UFaerieItemContainerBase* Container, const FEntryKey Key,
													   const int32 Removal)
{
	Super::PreRemoval(Container, Key, Removal);

	if (const FStructView ContainerData = FindFastArrayForContainer(Container);
		ContainerData.IsValid())
	{
		FRepDataFastArray& Ref = ContainerData.Get<FRepDataFastArray>();

		// If the whole stack is being removed, auto-delete any data we have for the entry
		if (Container->GetStack(Key) == Removal || Removal == Faerie::ItemData::UnlimitedStack)
		{
			Ref.RemoveDataForEntry(Key);
		}
	}
}

FConstStructView UInventoryReplicatedDataExtensionBase::GetDataForEntry(const UFaerieItemContainerBase* Container,
	const FEntryKey Key) const
{
	if (const FConstStructView ContainerData = FindFastArrayForContainer(Container);
		ContainerData.IsValid())
	{
		const FRepDataFastArray& Ref = ContainerData.Get<const FRepDataFastArray>();

		if (Ref.Contains(Key))
		{
			return Ref[Key];
		}
	}
	return FConstStructView(GetDataScriptStruct());
}

bool UInventoryReplicatedDataExtensionBase::EditDataForEntry(const UFaerieItemContainerBase* Container,
	const FEntryKey Key, const TFunctionRef<void(FStructView)>& Edit)
{
	const FStructView ContainerData = FindFastArrayForContainer(Container);
	if (!ContainerData.IsValid())
	{
		return false;
	}

	FRepDataFastArray& Ref = ContainerData.Get<FRepDataFastArray>();

	if (Ref.Contains(Key))
	{
		// Edit existing data entry
		Edit(Ref[Key]);
	}
	else
	{
		// Make new data entry
		FInstancedStruct Data(GetDataScriptStruct());
		Edit(Data);
		Ref.SetDataForEntry(Key, Data);
	}

	return true;
}

FStructView UInventoryReplicatedDataExtensionBase::FindFastArrayForContainer(const UFaerieItemContainerBase* Container)
{
	if (auto&& Found = PerContainerData.FindByPredicate(
			[Container](const TObjectPtr<URepDataArrayWrapper>& Userdata)
			{
				return Userdata && Userdata->Container == Container;
			}))
	{
		return FStructView::Make((*Found)->DataArray);
	}
	return nullptr;
}

FConstStructView UInventoryReplicatedDataExtensionBase::FindFastArrayForContainer(const UFaerieItemContainerBase* Container) const
{
	if (auto&& Found = PerContainerData.FindByPredicate(
			[Container](const TObjectPtr<URepDataArrayWrapper>& Userdata)
			{
				return Userdata && Userdata->Container == Container;
			}))
	{
		return FConstStructView::Make((*Found)->DataArray);
	}
	return nullptr;
}