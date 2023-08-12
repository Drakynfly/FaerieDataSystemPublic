// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "Subsystems/LocalPlayerSubsystem.h"
#include "ItemDragDropNotifications.generated.h"

class UFaerieItemDataProxyBase;

DECLARE_DYNAMIC_DELEGATE_OneParam(FItemDragDropNotificationCallback, UFaerieItemDataProxyBase*, ItemProxy);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FItemDragDropNotification, UFaerieItemDataProxyBase*, ItemProxy);

UENUM()
enum class EItemDragDropEvent : uint8
{
	MoveBegin,
	MoveEnd,
	HoverBegin,
	HoverEnd,

	MAX UMETA(Hidden)
};
ENUM_RANGE_BY_COUNT(EItemDragDropEvent, EItemDragDropEvent::MAX)


/**
 * User interface notifications about item proxies that can be drag-dropped between arbitrary UI interfaces for items.
 */
UCLASS()
class FAERIEINVENTORYCONTENT_API UItemDragDropNotifications : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION(BlueprintCallable)
	void SubscribeToEvent(const FItemDragDropNotificationCallback& Callback, EItemDragDropEvent Event);

	UFUNCTION(BlueprintCallable)
	void UnsubscribeFromEvent(const FItemDragDropNotificationCallback& Callback, EItemDragDropEvent Event);

	UFUNCTION(BlueprintCallable)
	void BroadcastEvent(UFaerieItemDataProxyBase* ItemProxy, EItemDragDropEvent Event);

private:
	FItemDragDropNotification* NotificationForEvent(EItemDragDropEvent Event);

	TMap<EItemDragDropEvent, FItemDragDropNotification> Notifications;
};
