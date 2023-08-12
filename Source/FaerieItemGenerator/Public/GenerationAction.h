// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "Engine/StreamableManager.h"
#include "GenerationAction.generated.h"

struct FFaerieItemSlotHandle;

DECLARE_LOG_CATEGORY_EXTERN(LogGenerationAction, Log, All)

UENUM(BlueprintType)
enum class EGenerationActionResult : uint8
{
	Failed,
	Timeout,
	Succeeded
};

using FNativeGenerationActionCompletedCallback = TDelegate<void(EGenerationActionResult)>;
DECLARE_DYNAMIC_DELEGATE_TwoParams(FGenerationActionOnCompleteBinding, EGenerationActionResult, Result, const TArray<UFaerieItemDataProxyBase*>&, Items);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FGenerationActionCompleted, EGenerationActionResult, Result, const TArray<UFaerieItemDataProxyBase*>&, Items);

USTRUCT(BlueprintType)
struct FGenerationActionSparseClassStruct
{
	GENERATED_BODY()

	FGenerationActionSparseClassStruct() {}

	// Flag for requiring a dedicated config function before this action can run.
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Configuration")
	bool RequiresExplicitConfiguration = true;

	// The maximum duration an action can run, in seconds, before timing out.
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Configuration")
	float DefaultTimeoutTime = 10.f;
};

class IFaerieItemSlotInterface;
class UFaerieItemDataProxyBase;
class UFaerieItemGeneratorSubsystem;

/**
 *
 */
UCLASS(Abstract, ClassGroup = "Faerie", Within = FaerieItemGeneratorSubsystem, SparseClassDataTypes = GenerationActionSparseClassStruct)
class FAERIEITEMGENERATOR_API UGenerationActionBase : public UObject
{
	GENERATED_BODY()

public:
	struct FActionArgs
	{
		TObjectPtr<UObject> Executor;
		FGenerationActionOnCompleteBinding Callback;
	};

	UGenerationActionBase();

	virtual UWorld* GetWorld() const override;

protected:
	// Optional overload, to preload assets before Run is executed.
	virtual TArray<FSoftObjectPath> GetAssetsToLoad() const { return {}; }

	// Internal virtual run function. This must be implemented per subclass. It must finish before the timer has ran out.
	virtual void Run() PURE_VIRTUAL(UGenerationActionBase::Run, )

private:
	FTimerManager& GetTimerManager() const;

	void OnTimeout();

	// This MUST be called during the actions execution or this action will be timed-out.
	void Finish(EGenerationActionResult Result);

protected:
	FStreamableManager& GetStreamableManager() { return StreamableManager; }

	// If subclasses leave RequiresExplicitConfiguration true, then this must be called before trying to Run.
	void Configure(FActionArgs& Args);

protected:
	UFUNCTION(BlueprintCallable, Category = "Invention Action Control")
	void Complete();

	UFUNCTION(BlueprintCallable, Category = "Invention Action Control")
	void Fail();

public:
	UFUNCTION(BlueprintCallable, Category = "Invention Action Control")
	void Start();

public:
	FNativeGenerationActionCompletedCallback OnCompletedCallback;

	UPROPERTY(BlueprintAssignable, Category = "Action Events")
	FGenerationActionCompleted OnCompleted;

protected:
	// The object who requested this action to run
	UPROPERTY()
	TObjectPtr<UObject> Executor;

	UPROPERTY()
	TArray<TObjectPtr<UFaerieItemDataProxyBase>> OutProxies;

private:
	UPROPERTY()
	FTimerHandle TimerHandle;

#if WITH_EDITORONLY_DATA
	// Running state of the action. All errors related to this should be discovered and resolved in the editor.
	UPROPERTY()
	bool IsRunning_EditorOnlyTracker = false;

	// Timestamp to record how long this action takes to run.
	UPROPERTY()
	FDateTime TimeStarted;
#endif

	FStreamableManager StreamableManager;
};

UCLASS(Abstract)
class FAERIEITEMGENERATOR_API UGenerationActionWithSlots : public UGenerationActionBase
{
	GENERATED_BODY()

protected:
	// Remove items and durability from the entries in Slots used to fund this action.
	void ConsumeSlotCosts(const IFaerieItemSlotInterface* Interface);

public:
	struct FActionArgs : Super::FActionArgs
	{
		bool RunConsumeStep = false;
		TMap<FFaerieItemSlotHandle, TObjectPtr<UFaerieItemDataProxyBase>> FilledSlots;
	};

	void Configure(FActionArgs& Args);

	UFUNCTION(BlueprintPure, Category = "Faerie|GenerationAction")
	UFaerieItemDataProxyBase* GetProxyForSlot(const FFaerieItemSlotHandle& Filter, bool ExpectPresence = false) const;

protected:
	// Should ConsumeSlotCosts be called during Run. This is disabled to preview output before commiting to the action.
	UPROPERTY()
	bool RunConsumeStep = false;

	// Crafting slots, and the inventory key being used to provide data to them.
	UPROPERTY()
	TMap<FFaerieItemSlotHandle, TObjectPtr<UFaerieItemDataProxyBase>> FilledSlots;
};