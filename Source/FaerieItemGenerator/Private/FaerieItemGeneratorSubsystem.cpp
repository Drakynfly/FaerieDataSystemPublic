// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "FaerieItemGeneratorSubsystem.h"

#include "FaerieItemDataFilter.h"
#include "FaerieItemTemplate.h"
#include "ItemCraftingConfig.h"
#include "ItemUpgradeConfig.h"

#include "GenerationAction.h"
#include "GenerationAction_CraftItems.h"
#include "GenerationAction_GenerateItems.h"
#include "GenerationAction_UpgradeItems.h"

DEFINE_LOG_CATEGORY(LogItemGeneratorSubsystem)

void UFaerieItemGeneratorSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	Initialized = true;
}

void UFaerieItemGeneratorSubsystem::BeginRunningAction(UGenerationActionBase* Action)
{
	check(Action);
	ActiveAction = Action;
	ActiveAction->OnCompletedCallback.BindUObject(this, &ThisClass::OnActionCompleted);
	ActiveAction->Start();
}

void UFaerieItemGeneratorSubsystem::EnqueueAction_Internal(UGenerationActionBase* NewAction)
{
	if (!IsValid(ActiveAction))
	{
		BeginRunningAction(NewAction);
		return;
	}

	PendingActions.Enqueue(TStrongObjectPtr<UGenerationActionBase>(NewAction));
}

void UFaerieItemGeneratorSubsystem::OnActionCompleted(EGenerationActionResult /*Result*/)
{
	if (!PendingActions.IsEmpty())
	{
		TStrongObjectPtr<UGenerationActionBase> Next;
		PendingActions.Dequeue(Next);
		if (Next.IsValid())
		{
			BeginRunningAction(Next.Get());
		}
	}
	else
	{
		ActiveAction = nullptr;
	}
}

void UFaerieItemGeneratorSubsystem::SubmitGenerationRequest(const FGenerationRequest& Request)
{
	if (!IsValid(Request.Executor))
	{
		UE_LOG(LogItemGeneratorSubsystem, Warning, __FUNCTION__ TEXT(": Executor is invalid!"));
		return;
	}

	if (Request.Drivers.IsEmpty())
	{
		UE_LOG(LogItemGeneratorSubsystem, Warning, __FUNCTION__ TEXT(": Drivers are empty!"));
		return;
	}

	for (auto&& Driver : Request.Drivers)
	{
		if (!Driver)
		{
			UE_LOG(LogItemGeneratorSubsystem, Warning, __FUNCTION__ TEXT(": Driver is misconfigured!"));
			return;
		}
	}

	UGenerationAction_GenerateItems::FActionArgs Args;
	Args.Executor = Request.Executor;
	Args.Callback = Request.OnComplete;
	Args.Drivers = Request.Drivers;

	EnqueueActionTyped<UGenerationAction_GenerateItems>(Args);
}

void UFaerieItemGeneratorSubsystem::SubmitUpgradeRequest(const FUpgradeRequest& Request)
{
	if (!IsValid(Request.Executor))
	{
		UE_LOG(LogItemGeneratorSubsystem, Warning, __FUNCTION__ TEXT(": Executor is invalid!"));
		return;
	}

	if (!IsValid(Request.ItemProxy))
	{
		UE_LOG(LogItemGeneratorSubsystem, Warning, __FUNCTION__ TEXT(": ItemProxy is invalid!"));
		return;
	}

	if (!IsValid(Request.Config))
	{
		UE_LOG(LogItemGeneratorSubsystem, Warning, __FUNCTION__ TEXT(": Config is invalid!"));
		return;
	}

	UGenerationAction_UpgradeItems::FActionArgs Args;
	Args.Executor = Request.Executor;
	Args.Callback = Request.OnComplete;
	Args.UpgradeConfig = Request.Config;
	Args.ItemBeingUpgraded = Request.ItemProxy;

	const FConstStructView SlotsView = UFaerieItemSlotLibrary::GetCraftingSlotsFromInterface(Request.Config);
	const FFaerieItemCraftingSlots* SlotsPtr = SlotsView.GetPtr<FFaerieItemCraftingSlots>();

	for (auto&& RequiredSlot : SlotsPtr->RequiredSlots)
	{
		if (auto&& SlotPtr = Request.Slots.FindByPredicate([RequiredSlot](const FRequestSlot& Slot)
		{
			return Slot.SlotID == RequiredSlot.Key;
		}))
		{
			if (!IsValid(SlotPtr->ItemProxy))
			{
				UE_LOG(LogItemGeneratorSubsystem, Warning, __FUNCTION__ TEXT(": Proxy is invalid for slot: %s!"), *RequiredSlot.Key.ToString());
				return;
			}

			if (RequiredSlot.Value->GetPattern()->Exec(SlotPtr->ItemProxy))
			{
				Args.FilledSlots.Add(RequiredSlot.Key, SlotPtr->ItemProxy);
			}
			else
			{
				UE_LOG(LogItemGeneratorSubsystem, Warning, __FUNCTION__ TEXT(": Required Slot '%s'failed with key: %s"),
					*SlotPtr->SlotID.ToString(), *SlotPtr->ItemProxy.GetName());
				return;
			}
		}
		else
		{
			UE_LOG(LogItemGeneratorSubsystem, Warning, __FUNCTION__ TEXT(": Request does not contain required slot: %s!"), *RequiredSlot.Key.ToString());
			return;
		}
	}

	for (auto&& OptionalSlot : SlotsPtr->OptionalSlots)
	{
		if (auto&& SlotPtr = Request.Slots.FindByPredicate([OptionalSlot](const FRequestSlot& Slot)
		{
			return Slot.SlotID == OptionalSlot.Key;
		}))
		{
			if (!IsValid(SlotPtr->ItemProxy))
			{
				UE_LOG(LogItemGeneratorSubsystem, Warning, __FUNCTION__ TEXT(": Entry is invalid for slot: %s!"), *OptionalSlot.Key.ToString());
				return;
			}

			if (OptionalSlot.Value->GetPattern()->Exec(SlotPtr->ItemProxy))
			{
				Args.FilledSlots.Add(OptionalSlot.Key, SlotPtr->ItemProxy);
			}
			else
			{
				UE_LOG(LogItemGeneratorSubsystem, Warning, __FUNCTION__ TEXT(": Optional Slot '%s' failed with key: %s"),
					*SlotPtr->SlotID.ToString(), *SlotPtr->ItemProxy.GetName());
				return;
			}
		}
	}

	// With all slots validated, execute the upgrade.
	EnqueueActionTyped<UGenerationAction_UpgradeItems>(Args);
}

void UFaerieItemGeneratorSubsystem::SubmitCraftingRequest_Impl(const FCraftingRequest& Request, const bool Preview)
{
	if (!IsValid(Request.Executor))
	{
		UE_LOG(LogItemGeneratorSubsystem, Warning, __FUNCTION__ TEXT(": Executor is invalid!"));
		return;
	}

	if (!IsValid(Request.Config))
	{
		UE_LOG(LogItemGeneratorSubsystem, Warning, __FUNCTION__ TEXT(": Config is invalid!"));
		return;
	}

	if (!IsValid(Request.Executor))
	{
		UE_LOG(LogItemGeneratorSubsystem, Warning, __FUNCTION__ TEXT(": ItemOuter is invalid!"));
		return;
	}

	UGenerationAction_CraftItems::FActionArgs Args;
	Args.Executor = Request.Executor;
	Args.Callback = Request.OnComplete;
	Args.CraftConfig = Request.Config;
	Args.RunConsumeStep = !Preview;

	const FConstStructView SlotsView = UFaerieItemSlotLibrary::GetCraftingSlotsFromInterface(Request.Config);
	const FFaerieItemCraftingSlots* SlotsPtr = SlotsView.GetPtr<FFaerieItemCraftingSlots>();

	for (auto&& RequiredSlot : SlotsPtr->RequiredSlots)
	{
		if (auto&& SlotPtr = Request.Slots.FindByPredicate([RequiredSlot](const FRequestSlot& Slot)
		{
			return Slot.SlotID == RequiredSlot.Key;
		}))
		{
			if (!IsValid(SlotPtr->ItemProxy))
			{
				UE_LOG(LogItemGeneratorSubsystem, Warning, __FUNCTION__ TEXT(": Entry is invalid for slot: %s!"), *RequiredSlot.Key.ToString());
				return;
			}

			if (RequiredSlot.Value->GetPattern()->Exec(SlotPtr->ItemProxy))
			{
				Args.FilledSlots.Add(RequiredSlot.Key, SlotPtr->ItemProxy);
			}
			else
			{
				UE_LOG(LogItemGeneratorSubsystem, Warning, __FUNCTION__ TEXT(": Required Slot '%s' failed with key: %s"),
					*SlotPtr->SlotID.ToString(), *SlotPtr->ItemProxy.GetName());
				return;
			}
		}
		else
		{
			UE_LOG(LogItemGeneratorSubsystem, Warning, __FUNCTION__ TEXT(": Request does contain required slot: %s!"), *RequiredSlot.Key.ToString());
			return;
		}
	}

	for (auto&& OptionalSlot : SlotsPtr->OptionalSlots)
	{
		if (auto&& SlotPtr = Request.Slots.FindByPredicate([OptionalSlot](const FRequestSlot& Slot)
		{
			return Slot.SlotID == OptionalSlot.Key;
		}))
		{
			if (!IsValid(SlotPtr->ItemProxy))
			{
				UE_LOG(LogItemGeneratorSubsystem, Warning, __FUNCTION__ TEXT(": Entry is invalid for slot: %s!"), *OptionalSlot.Key.ToString());
				return;
			}

			if (OptionalSlot.Value->GetPattern()->Exec(SlotPtr->ItemProxy))
			{
				Args.FilledSlots.Add(OptionalSlot.Key, SlotPtr->ItemProxy);
			}
			else
			{
				UE_LOG(LogItemGeneratorSubsystem, Warning, __FUNCTION__ TEXT("Optional Slot '%s' failed with key: %s"),
					*SlotPtr->SlotID.ToString(), *SlotPtr->ItemProxy.GetName());
				return;
			}
		}
	}

	// With all slots validated, execute the upgrade.
	EnqueueActionTyped<UGenerationAction_CraftItems>(Args);
}

void UFaerieItemGeneratorSubsystem::SubmitCraftingRequest(const FCraftingRequest& Request)
{
	SubmitCraftingRequest_Impl(Request, false);
}

void UFaerieItemGeneratorSubsystem::PreviewCraftingRequest(const FCraftingRequest& Request)
{
	SubmitCraftingRequest_Impl(Request, true);
}