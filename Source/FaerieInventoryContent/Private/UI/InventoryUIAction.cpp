// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "UI/InventoryUIAction.h"
#include "UI/InventoryContentsBase.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(InventoryUIAction)

DEFINE_LOG_CATEGORY(LogInventoryUIAction)

UWorld* UInventoryUIAction::GetWorld() const
{
	if (ContentsWidget)
	{
		return ContentsWidget->GetWorld();
	}
	return nullptr;
}

void UInventoryUIAction::Setup_Implementation(UInventoryContentsBase* InContentsWidget)
{
	ContentsWidget = InContentsWidget;
}

void UInventoryUIAction::Finish()
{
	if (!InProgress)
	{
		UE_LOG(LogInventoryUIAction, Error, TEXT("Action cannot finish. Is not in progress!"))
		return;
	}

	InProgress = false;
}

EInventoryUIActionState UInventoryUIAction::CanRunOnEntry_Implementation(const FInventoryKey InKey) const
{
	return EInventoryUIActionState::Enabled;
}

bool UInventoryUIAction::Start(const FInventoryKey InKey)
{
	if (!ContentsWidget)
	{
		UE_LOG(LogInventoryUIAction, Error, TEXT("ContentsWidget has not been set. Action will not run!"))
		return false;
	}

	if (InProgress)
	{
		UE_LOG(LogInventoryUIAction, Error, TEXT("Action already in progress!"))
		return false;
	}

	Key = InKey;
	InProgress = true;
	Run();
	return true;
}

void UInventoryUIAction::Run_Implementation()
{
	checkf(0, TEXT("Override this!"));
}