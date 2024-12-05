// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "Actions/FaerieInventoryClient.h"
#include "FaerieItemStorage.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FaerieInventoryClient)

UFaerieInventoryClient::UFaerieInventoryClient()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UFaerieInventoryClient::CanAccessContainer(const UFaerieItemContainerBase* Container) const
{
	// @todo implement
	/*
	if (auto&& PermissionExtensions = Storage->GetExtension<UInventoryClientPermissionExtensions>())
	{
		if (!PermissionExtensions->AllowsClient(this))
		{
			return false;
		}
	}
	*/
	return true;
}

bool UFaerieInventoryClient::CanAccessStorage(const UFaerieItemStorage* Storage) const
{
	return CanAccessContainer(Storage);
}

void UFaerieInventoryClient::RequestExecuteAction_Implementation(const TInstancedStruct<FFaerieClientActionBase>& Args)
{
	if (Args.IsValid())
	{
		(void)Args.Get().Server_Execute(this);
	}
}

void UFaerieInventoryClient::RequestExecuteAction_Batch_Implementation(
	const TArray<TInstancedStruct<FFaerieClientActionBase>>& Args, const EFaerieClientRequestBatchType Type)
{
	for (auto&& Element : Args)
	{
		bool Ran = false;
		if (Element.IsValid())
		{
			Ran = Element.Get().Server_Execute(this);
		}

		if (!Ran && Type == EFaerieClientRequestBatchType::Sequence)
		{
			// Sequence failed, exit.
			return;
		}
	}
}

bool FFaerieClientAction_RequestDeleteEntry::Server_Execute(const UFaerieInventoryClient* Client) const
{
	auto&& Storage = Handle.ItemStorage.Get();
	if (!IsValid(Storage)) return false;
	if (!Client->CanAccessStorage(Storage)) return false;

	return Storage->RemoveStack(Handle.Key, Faerie::Inventory::Tags::RemovalDeletion, Amount);
}

bool FFaerieClientAction_RequestMoveEntry::Server_Execute(const UFaerieInventoryClient* Client) const
{
	auto&& Storage = Handle.ItemStorage.Get();
	if (!IsValid(Storage)) return false;
	if (!IsValid(ToStorage)) return false;
	if (!Client->CanAccessStorage(Storage)) return false;
	if (Storage == ToStorage) return false;

	return Storage->MoveStack(ToStorage, Handle.Key, Amount).IsValid();
}

bool FFaerieClientAction_RequestSplitStack::Server_Execute(const UFaerieInventoryClient* Client) const
{
	if (!IsValid(Storage)) return false;
	if (!Client->CanAccessStorage(Storage)) return false;
	return Storage->SplitStack(Key.EntryKey, Key.StackKey, Amount);
}