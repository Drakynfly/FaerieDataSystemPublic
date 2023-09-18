// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "ItemContainerExtensionBase.h"
#include "FaerieItemContainerBase.h"
#include "ItemContainerEvent.h"
#include "InventoryLoggerExtension.generated.h"

USTRUCT(BlueprintType, meta = (HasNativeBreak = "/Script/FaerieInventoryContent.LoggedInventoryEventLibrary.BreakLoggedInventoryEvent"))
struct FLoggedInventoryEvent
{
	GENERATED_BODY()

	// Which storage logged this event
	UPROPERTY()
	TWeakObjectPtr<const UFaerieItemContainerBase> Container = nullptr;

	// The logged event
	Faerie::Inventory::FEventLog Event;

	friend bool operator==(const FLoggedInventoryEvent& Lhs, const FLoggedInventoryEvent& Rhs)
	{
		return Lhs.Container == Rhs.Container && Lhs.Event == Rhs.Event;
	}

	friend bool operator!=(const FLoggedInventoryEvent& Lhs, const FLoggedInventoryEvent& Rhs)
	{
		return !(Lhs == Rhs);
	}

	friend FArchive& operator<<(FArchive& Ar, FLoggedInventoryEvent& Val)
	{
		Ar << Val.Container;
		Ar << Val.Event;
		return Ar;
	}

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
	{
		Ar << *this;
		bOutSuccess = true;
		return true;
	}
};

template<>
struct TStructOpsTypeTraits<FLoggedInventoryEvent> : public TStructOpsTypeTraitsBase2<FLoggedInventoryEvent>
{
	enum
	{
		WithNetSerializer = true,
	};
};


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInventoryEventLogged, const FLoggedInventoryEvent&, LoggedEvent);

/**
 * Logs events from additions, changes, and removals, and can parse them for data at request.
 */
UCLASS()
class FAERIEINVENTORYCONTENT_API UInventoryLoggerExtension : public UItemContainerExtensionBase
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void PostAddition(const UFaerieItemContainerBase* Container, const Faerie::Inventory::FEventLog& Event) override;
	virtual void PostRemoval(const UFaerieItemContainerBase* Container, const Faerie::Inventory::FEventLog& Event) override;

	void HandleNewEvent(const FLoggedInventoryEvent& Event);

public:
	UFUNCTION(BlueprintCallable, Category = "LoggerExtension")
	int32 GetNumEvents() const { return EventLog.Num(); }

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "LoggerExtension")
	const TArray<FLoggedInventoryEvent>& GetAllEvents() const { return EventLog; }

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "LoggerExtension")
	TArray<FLoggedInventoryEvent> GetRecentEvents(int32 NumEvents) const;

	UPROPERTY(BlueprintAssignable, Category = "LoggerExtension|Events")
	FInventoryEventLogged OnInventoryEventLogged;

protected:
	UFUNCTION()
	virtual void OnRep_EventLog();


	UPROPERTY(ReplicatedUsing = "OnRep_EventLog")
	TArray<FLoggedInventoryEvent> EventLog;

private:
	int32 LocalEventLogCount = 0;
};

UCLASS()
class ULoggedInventoryEventLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "LoggedInventoryEventLibrary", meta = (NativeBreakFunc))
	static void BreakLoggedInventoryEvent(const FLoggedInventoryEvent& LoggedEvent, FFaerieInventoryTag& Type, bool& Success,
										  FDateTime& Timestamp, FEntryKey& EntryTouched, TArray<FFaerieItemKeyBase>& OtherKeysTouched,
										  FFaerieItemStackView& Stack, FString& ErrorMessage);
};