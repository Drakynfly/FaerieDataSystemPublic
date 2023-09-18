// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "ItemContainerExtensionBase.h"
#include "CapacityStructs.h"
#include "InventoryCapacityExtension.generated.h"

UENUM(BlueprintType, Meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class ECapacityChecks : uint8
{
    None    = 0 UMETA(Hidden),
    Bounds  = 1 << 0,
    Weight  = 1 << 1,
    Volume  = 1 << 2,
};
ENUM_CLASS_FLAGS(ECapacityChecks)

USTRUCT(BlueprintType)
struct FCapacityExtensionConfig
{
    GENERATED_BODY()

    /** Which capacity checks are performed when determining if an entry can "fit" in the inventory. */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Capacity Config", meta = (Bitmask, BitmaskEnum = "/Script/FaerieInventoryContent.ECapacityChecks"))
    int32 Checks = 0;

    /** Size in centimeters of the maximum bounding box for contained items. */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Capacity Config", meta = (Units = cm))
    FIntVector Bounds = FIntVector(0);

    /** How much items can exceed the bounds, but still be allowed to be contained. Useful for soft containers */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Capacity Config")
    float BoundsFudgeFactor = 1.1f;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Capacity Config")
    FWeightEditor MaxWeight = 0.f;

    /** If enabled, MaxVolume will be set to Bounds.X*Bounds.Y*Bounds.Z. Disable to manually edit MaxVolume */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Capacity Config")
    bool DeriveVolumeFromBounds = true;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Capacity Config", meta = (EditCondition = "!DeriveVolumeFromBounds"))
    int64 MaxVolume = 0.f;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Capacity Config")
    bool AllowEntriesWithNoCapacityToken = false;

    bool HasCheck(const ECapacityChecks Check) const
    {
        return EnumHasAnyFlags(static_cast<ECapacityChecks>(Checks), Check);
    }
};

/**
 * The state of the capacity that is replicated to all clients.
 */
USTRUCT(BlueprintType)
struct FCapacityExtensionState
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly, Category = "Weight", meta = (Units = g))
    int32 CurrentWeight = 0.f;

    UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly, Category = "Volume")
    int64 CurrentVolume = 0.f;

    UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly, Category = "Over Max Status", meta = (AllowPrivateAccess = true))
    bool OverMaxWeight = false;

    UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly, Category = "Over Max Status", meta = (AllowPrivateAccess = true))
    bool OverMaxVolume = false;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FInventoryCapacityEvent);

/**
 * This class is the parser for extracting a Capacity out from Item Data.
 */
UCLASS()
class FAERIEINVENTORYCONTENT_API UInventoryCapacityExtension : public UItemContainerExtensionBase
{
    GENERATED_BODY()

public:
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
    virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent) override;
#endif

protected:
    //~ UItemContainerExtensionBase
    virtual void InitializeExtension(const UFaerieItemContainerBase* Container) override;
    virtual void DeinitializeExtension(const UFaerieItemContainerBase* Container) override;
    virtual EEventExtensionResponse AllowsAddition(const UFaerieItemContainerBase* Container, FFaerieItemStackView Stack) override;
    virtual void PostAddition(const UFaerieItemContainerBase* Container, const Faerie::Inventory::FEventLog& Event) override;
    virtual void PostRemoval(const UFaerieItemContainerBase* Container, const Faerie::Inventory::FEventLog& Event) override;
    virtual void PostEntryChanged(const UFaerieItemContainerBase* Container, const FEntryKey Key) override;
    //~ UItemContainerExtensionBase

private:
    static FWeightAndVolume GetEntryWeightAndVolume(const UFaerieItemContainerBase* Container, const FEntryKey Key);

    void UpdateCacheForEntry(const UFaerieItemContainerBase* Container, FEntryKey Key);

    void CheckCapacityLimit();

    bool CanContainToken(const class UFaerieCapacityToken* Token, const int32 Stack) const;

    void AddWeightAndVolume(FWeightAndVolume Value);

    void HandleStateChanged();

public:
    // Tests if the capacity of an entry can fit in this container.
    UFUNCTION(BlueprintPure, Category = "Faerie|InventoryCapacity")
    bool CanContain(FFaerieItemStackView Stack) const;

    // Tests if the capacity of an item can fit in this container.
    UFUNCTION(BlueprintPure, Category = "Faerie|InventoryCapacity")
    bool CanContainProxy(FFaerieItemProxy Proxy) const;

    // Get the configuration struct.
    UFUNCTION(BlueprintPure, Category = "Faerie|InventoryCapacity")
    const FCapacityExtensionConfig& GetCapacityConfig() const { return Config; }

    // Get the current state struct.
    UFUNCTION(BlueprintPure, Category = "Faerie|InventoryCapacity")
    FCapacityExtensionState GetCurrentState() const { return State; }

    // Get the current amount filled.
    UFUNCTION(BlueprintPure, Category = "Faerie|InventoryCapacity")
    FWeightAndVolume GetCurrentCapacity() const;

    // Get the maximum amount that this manager can hold.
    UFUNCTION(BlueprintPure, Category = "Faerie|InventoryCapacity")
    FWeightAndVolume GetMaxCapacity() const;

    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Faerie|InventoryCapacity")
    void SetConfiguration(const FCapacityExtensionConfig& NewConfig);

    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Faerie|InventoryCapacity")
    void SetMaxCapacity(const FWeightAndVolume NewMax);

    UFUNCTION(BlueprintPure, Category = "Faerie|InventoryCapacity")
    float GetPercentageFullForWeightAndVolume(const FWeightAndVolume& WeightAndVolume) const;

    // Get our current percentage "fullness"
    UFUNCTION(BlueprintPure, Category = "Faerie|InventoryCapacity")
    float GetPercentageFull() const;

    // Broadcast whenever the state changes.
    UPROPERTY(BlueprintCallable, BlueprintAssignable, Category = "Events")
    FInventoryCapacityEvent OnStateChanged;

    // Broadcast whenever the config changes.
    UPROPERTY(BlueprintCallable, BlueprintAssignable, Category = "Events")
    FInventoryCapacityEvent OnConfigurationChanged;

protected:
    UFUNCTION()
    virtual void OnRep_Config();

    UFUNCTION()
    virtual void OnRep_State();

protected:
    UPROPERTY(ReplicatedUsing = "OnRep_Config", EditAnywhere, Category = "Capacity", meta = (ShowOnlyInnerProperties))
    FCapacityExtensionConfig Config;

    UPROPERTY(ReplicatedUsing = "OnRep_State", VisibleInstanceOnly, Category = "Capacity")
    FCapacityExtensionState State;

    // Cache of all entries to maintain serverside integrity.
    // @todo actually use this to validate State
    TMap<TWeakObjectPtr<const UFaerieItemContainerBase>, TMap<FEntryKey, FWeightAndVolume>> ServerCapacityCache;
};