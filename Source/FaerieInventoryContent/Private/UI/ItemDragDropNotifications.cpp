// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "UI/ItemDragDropNotifications.h"

void UItemDragDropNotifications::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	for (EItemDragDropEvent Event : TEnumRange<EItemDragDropEvent>())
	{
		Notifications.Add(Event, FItemDragDropNotification());
	}
}

void UItemDragDropNotifications::SubscribeToEvent(const FItemDragDropNotificationCallback& Callback,
                                                  const EItemDragDropEvent Event)
{
	TScriptDelegate Delegate;
	Delegate.BindUFunction(const_cast<UObject*>(Callback.GetUObject()), Callback.GetFunctionName());
	if (auto&& Notification = NotificationForEvent(Event))
	{
		Notification->Add(Delegate);
	}
}

void UItemDragDropNotifications::UnsubscribeFromEvent(const FItemDragDropNotificationCallback& Callback,
                                                      const EItemDragDropEvent Event)
{
	if (auto&& Notification = NotificationForEvent(Event))
	{
		Notification->Remove(Callback);
	}
}

void UItemDragDropNotifications::BroadcastEvent(UFaerieItemDataProxyBase* ItemProxy, const EItemDragDropEvent Event)
{
	if (auto&& Notification = NotificationForEvent(Event))
	{
		Notification->Broadcast(ItemProxy);
	}
}

FItemDragDropNotification* UItemDragDropNotifications::NotificationForEvent(const EItemDragDropEvent Event)
{
	return Notifications.Find(Event);
}
