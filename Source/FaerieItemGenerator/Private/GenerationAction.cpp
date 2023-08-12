// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "GenerationAction.h"

#include "FaerieItem.h"
#include "FaerieItemDataProxy.h"
#include "FaerieItemGeneratorSubsystem.h"
#include "FaerieItemSlotUtils.h"
#include "Algo/ForEach.h"
#include "Tokens/FaerieItemUsesToken.h"

DEFINE_LOG_CATEGORY(LogGenerationAction)

UGenerationActionBase::UGenerationActionBase()
{
}

UWorld* UGenerationActionBase::GetWorld() const
{
	return GetOuterUFaerieItemGeneratorSubsystem()->GetWorld();
}

FTimerManager& UGenerationActionBase::GetTimerManager() const
{
	auto&& World = GetWorld();
	return World->GetTimerManager();
}

void UGenerationActionBase::OnTimeout()
{
#if WITH_EDITORONLY_DATA
	if (!IsRunning_EditorOnlyTracker)
	{
		UE_LOG(LogGenerationAction, Error, TEXT("UGenerationActionBase::OnTimeout() called without IsRunning true!"))
	}
#endif

	Finish(EGenerationActionResult::Timeout);
}

void UGenerationActionBase::Finish(const EGenerationActionResult Result)
{
#if WITH_EDITORONLY_DATA
	if (!IsRunning_EditorOnlyTracker)
	{
		UE_LOG(LogGenerationAction, Error, TEXT("Finish(...) was called on Generation Action without IsRunning true!"));
		return;
	}

	IsRunning_EditorOnlyTracker = false;

	const FDateTime TimeFinished = FDateTime::UtcNow();
	const FTimespan TimePassed = TimeFinished - TimeStarted;

	switch (Result) {
	case EGenerationActionResult::Failed:
		UE_LOG(LogGenerationAction, Error, TEXT("+==+ Generation Action \"%s\" failed at: %s. Time passed: %s"), *GetName(), *TimeFinished.ToString(), *TimePassed.ToString());
		break;
	case EGenerationActionResult::Timeout:
		UE_LOG(LogGenerationAction, Warning, TEXT("+==+ Generation Action \"%s\" timed-out at: %s. Time passed: %s"), *GetName(), *TimeFinished.ToString(), *TimePassed.ToString());
		break;
	case EGenerationActionResult::Succeeded:
		UE_LOG(LogGenerationAction, Log, TEXT("+==+ Generation Action \"%s\" succeeded at: %s. Time passed: %s"), *GetName(), *TimeFinished.ToString(), *TimePassed.ToString());
		break;
	default: ; }
#endif

	GetTimerManager().ClearTimer(TimerHandle);

	OnCompleted.Broadcast(Result, OutProxies);
	OnCompletedCallback.ExecuteIfBound(Result);
}

void UGenerationActionBase::Configure(FActionArgs& Args)
{
	check(Args.Executor);

	Executor = Args.Executor;
	TScriptDelegate Delegate;
	Delegate.BindUFunction(Args.Callback.GetUObject(), Args.Callback.GetFunctionName());
	OnCompleted.Add(Delegate);
}

void UGenerationActionBase::Start()
{
#if WITH_EDITORONLY_DATA
	if (IsRunning_EditorOnlyTracker)
	{
		UE_LOG(LogGenerationAction, Error, TEXT("Start() called on Generation Action that is already running!"))
		return Fail();
	}

	IsRunning_EditorOnlyTracker = true;

	TimeStarted = FDateTime::UtcNow();

	UE_LOG(LogGenerationAction, Log, TEXT("+==+ Generation Action \"%s\" started at: %s"), *GetName(), *TimeStarted.ToString());
#endif

	GetTimerManager().SetTimer(TimerHandle,
		FTimerDelegate::CreateUObject(this, &ThisClass::OnTimeout), GetDefaultTimeoutTime(), false);

	TArray<FSoftObjectPath> ObjectsToLoad = GetAssetsToLoad();

	if (ObjectsToLoad.IsEmpty())
	{
		return Run();
	}

	UE_LOG(LogGenerationAction, Log, TEXT("- Objects to load: %i"), ObjectsToLoad.Num());

	// The check to HasBegunPlay forces this action to be ran in the editor synchronously
	if (GetWorld()->HasBegunPlay())
	{
		// Suspend generation to async load drop assets, then continue
		const FStreamableDelegate Delegate = FStreamableDelegate::CreateUObject(this, &ThisClass::Run);
		StreamableManager.RequestAsyncLoad(ObjectsToLoad, Delegate);
	}
	else
	{
		// Immediately load all objects and continue.
		for (FSoftObjectPath Object : ObjectsToLoad)
		{
			Object.TryLoad();
		}

		Run();
	}
}

void UGenerationActionBase::Complete()
{
	Finish(EGenerationActionResult::Succeeded);
}

void UGenerationActionBase::Fail()
{
	Finish(EGenerationActionResult::Failed);
}

UFaerieItemDataProxyBase* UGenerationActionWithSlots::GetProxyForSlot(const FFaerieItemSlotHandle& Filter, const bool ExpectPresence) const
{
	if (auto&& Key = FilledSlots.Find(Filter))
	{
		return *Key;
	}

	if (ExpectPresence)
	{
		UE_LOG(LogGenerationAction, Error,
			TEXT("<GetKeyForFilter> FilledSlots does not contain an entry for the requested slot ID: \'%s\'"), *Filter.ToString())
	}

	return nullptr;
}

void UGenerationActionWithSlots::ConsumeSlotCosts(const IFaerieItemSlotInterface* Interface)
{
	const FConstStructView SlotsView = UFaerieItemSlotLibrary::GetCraftingSlotsFromInterface(Interface);
	const FFaerieItemCraftingSlots* SlotsPtr = SlotsView.GetPtr<FFaerieItemCraftingSlots>();

	auto CanEat = [this](const TPair<FFaerieItemSlotHandle, TObjectPtr<UFaerieItemTemplate>>& Slot)
		{
			const bool HasSlotBeenFilled = FilledSlots.Contains(Slot.Key);
			if (!ensure(HasSlotBeenFilled))
			{
				UE_LOG(LogGenerationAction, Error, TEXT("ConsumeSlotCosts was unable to find a filled slot!"))
			}
			return HasSlotBeenFilled;
		};

	auto EatUse = [this](const TPair<FFaerieItemSlotHandle, TObjectPtr<UFaerieItemTemplate>>& Slot)
		{
			auto&& ItemProxy = *FilledSlots.Find(Slot.Key);

			const UFaerieItem* Item = ItemProxy->GetItemObject();

			bool RemovedUse = false;

			// If the item can be used as a resource multiple times.
			if (Item->IsInstanceMutable())
			{
				if (auto&& Uses = Item->GetToken<UFaerieItemUsesToken>())
				{
					RemovedUse = Uses->RemoveUses(1);
				}
			}

			// Otherwise, consume the item itself
			if (!RemovedUse)
			{
				ItemProxy->GetOwner()->Release({Item, 1});
			}
		};

	Algo::ForEachIf(SlotsPtr->RequiredSlots, CanEat, EatUse);
	Algo::ForEachIf(SlotsPtr->OptionalSlots, CanEat, EatUse);
}

void UGenerationActionWithSlots::Configure(FActionArgs& Args)
{
	RunConsumeStep = Args.RunConsumeStep;
	FilledSlots = Args.FilledSlots;
	Super::Configure(Args);
}