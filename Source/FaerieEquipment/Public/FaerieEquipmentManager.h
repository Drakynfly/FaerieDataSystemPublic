// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "EquipmentHashTypes.h"
#include "FaerieSlotTag.h"
#include "Components/ActorComponent.h"

#include "FaerieEquipmentManager.generated.h"

class UItemContainerExtensionGroup;
class UFaerieEquipmentSlot;
class UFaerieEquipmentSlotDescription;
class UItemContainerExtensionBase;

DECLARE_LOG_CATEGORY_EXTERN(LogEquipmentManager, Log, All)

UENUM(BlueprintType)
enum class EFaerieEquipmentClientChecksumState : uint8
{
	Desynchronized,
	Synchronized
};


using FEquipmentChangedEventNative = TMulticastDelegate<void(UFaerieEquipmentSlot*)>;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEquipmentChangedEvent, UFaerieEquipmentSlot*, Slot);
using FEquipmentClientChecksumEventNative = TMulticastDelegate<void(EFaerieEquipmentClientChecksumState)>;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEquipmentClientChecksumEvent, EFaerieEquipmentClientChecksumState, State);

UCLASS(Blueprintable, ClassGroup = ("Faerie"), meta = (BlueprintSpawnableComponent),
	HideCategories = (Collision, ComponentTick, Replication, ComponentReplication, Activation, Sockets, Navigation))
class FAERIEEQUIPMENT_API UFaerieEquipmentManager : public UActorComponent
{
	GENERATED_BODY()

	friend UFaerieEquipmentSlot;

public:
	UFaerieEquipmentManager();

	//~ UObject
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	//~ UObject

	//~ UActorComponent
	virtual void InitializeComponent() override;
	virtual void OnComponentCreated() override;
	virtual void ReadyForReplication() override;
	//~ UActorComponent

protected:
	void AddDefaultSlots();

	void OnSlotItemChanged(UFaerieEquipmentSlot* Slot);

	void RecalcLocalChecksum();

	void CheckLocalChecksum();

	UFUNCTION(/* Replication */)
	void OnRep_ServerChecksum();

public:
	/**------------------------------*/
	/*			SLOTS API			 */
	/**------------------------------*/

	FEquipmentChangedEventNative::RegistrationType& GetOnEquipmentSlotAdded() { return OnEquipmentSlotAddedNative; }
	FEquipmentChangedEventNative::RegistrationType& GetOnPreEquipmentSlotRemoved() { return OnPreEquipmentSlotRemovedNative; }
	FEquipmentChangedEventNative::RegistrationType& GetOnEquipmentChangedEvent() { return OnEquipmentChangedEventNative; }

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Faerie|EquipmentManager")
	UFaerieEquipmentSlot* AddSlot(FFaerieSlotTag SlotID, UFaerieEquipmentSlotDescription* Description);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Faerie|EquipmentManager")
	bool RemoveSlot(UFaerieEquipmentSlot* Slot);

	UFUNCTION(BlueprintCallable, Category = "Faerie|EquipmentManager")
	TArray<UFaerieEquipmentSlot*> GetSlots() const { return Slots; }

	/**
	 * Find a slot contained in this manager. Enable recursive to check slots contained in other slots.
	 */
	UFUNCTION(BlueprintCallable, Category = "Faerie|EquipmentManager")
	UFaerieEquipmentSlot* FindSlot(FFaerieSlotTag SlotID, bool Recursive = false) const;


	/**------------------------------*/
	/*		 EXTENSIONS SYSTEM		 */
	/**------------------------------*/

	UFUNCTION(BlueprintCallable, Category = "Storage|EquipmentManager")
	UItemContainerExtensionGroup* GetExtensions() const { return ExtensionGroup; }

	// Has extension by class. This only checks for extensions applied to *all* slots. Slot-specific extensions must be
	// requested from the slot itself using FindSlot + HasExtension
	UFUNCTION(BlueprintCallable, Category = "Storage|EquipmentManager")
	bool HasExtension(TSubclassOf<UItemContainerExtensionBase> ExtensionClass) const;

	// Gets an extension by class. This only checks for extensions applied to *all* slots. Slot-specific extensions must be
	// requested from the slot itself using FindSlot + GetExtension
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Faerie|EquipmentManager",
	meta = (DeterminesOutputType = ExtensionClass, DynamicOutputParam = Extension, ExpandBoolAsExecs = "ReturnValue"))
	bool GetExtension(UPARAM(meta = (AllowAbstract = "false")) TSubclassOf<UItemContainerExtensionBase> ExtensionClass,
		UItemContainerExtensionBase*& Extension) const;

	// Add a new extension of the given class, and return the result. If an extension of this class already exists, it
	// will be returned instead.
	UFUNCTION(BlueprintCallable, Category = "Faerie|EquipmentManager", BlueprintAuthorityOnly, meta = (DeterminesOutputType = "ExtensionClass"))
	UItemContainerExtensionBase* AddExtension(TSubclassOf<UItemContainerExtensionBase> ExtensionClass);

	// Removes any existing extension(s) of the given class.
	UFUNCTION(BlueprintCallable, Category = "Faerie|EquipmentManager", BlueprintAuthorityOnly)
	bool RemoveExtension(TSubclassOf<UItemContainerExtensionBase> ExtensionClass);

	// Add a new extension of the given class, and return the result. If an extension of this class already exists, it
	// will be returned instead. Adds only to the slot at the ID
	UFUNCTION(BlueprintCallable, Category = "Faerie|EquipmentManager", BlueprintAuthorityOnly, meta = (DeterminesOutputType = "ExtensionClass"))
	UItemContainerExtensionBase* AddExtensionToSlot(FFaerieSlotTag SlotID, TSubclassOf<UItemContainerExtensionBase> ExtensionClass);

	// Removes any existing extension(s) of the given class.
	UFUNCTION(BlueprintCallable, Category = "Faerie|EquipmentManager", BlueprintAuthorityOnly)
	bool RemoveExtensionFromSlot(FFaerieSlotTag SlotID, TSubclassOf<UItemContainerExtensionBase> ExtensionClass);


	/**------------------------------*/
	/*		 EQUIPMENT CHECKSUM		 */
	/**------------------------------*/

	UFUNCTION(BlueprintCallable, Category = "Faerie|EquipmentManager")
	FFaerieEquipmentHash GetLocalChecksum() const { return LocalChecksum; }

	UFUNCTION(BlueprintCallable, Category = "Faerie|EquipmentManager")
	FFaerieEquipmentHash GetServerChecksum() const { return ServerChecksum; }

	FEquipmentClientChecksumEventNative::RegistrationType& GetOnClientChecksumEvent() { return OnClientChecksumEventNative; }

protected:
	UPROPERTY(BlueprintAssignable, Transient, Category = "Events")
	FEquipmentClientChecksumEvent OnClientChecksumEvent;

	UPROPERTY(BlueprintAssignable, Transient, Category = "Events")
	FEquipmentChangedEvent OnEquipmentSlotAdded;

	UPROPERTY(BlueprintAssignable, Transient, Category = "Events")
	FEquipmentChangedEvent OnPreEquipmentSlotRemoved;

	// A generic event when any slot is changed, either by adding or removing the item, or the item itself is changed.
	UPROPERTY(BlueprintAssignable, Transient, Category = "Events")
	FEquipmentChangedEvent OnEquipmentChangedEvent;

private:
	FEquipmentClientChecksumEventNative OnClientChecksumEventNative;
	FEquipmentChangedEventNative OnEquipmentSlotAddedNative;
	FEquipmentChangedEventNative OnPreEquipmentSlotRemovedNative;
	FEquipmentChangedEventNative OnEquipmentChangedEventNative;

protected:
	UPROPERTY(EditAnywhere, Category = "Equipment")
	TMap<FFaerieSlotTag, TObjectPtr<UFaerieEquipmentSlotDescription>> DefaultSlots;

	// Predefined extensions added to all slots in this manager.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Instanced, NoClear, Category = "Extensions")
	TObjectPtr<UItemContainerExtensionGroup> ExtensionGroup;

private:
	UPROPERTY(Replicated, Transient)
	TArray<TObjectPtr<UFaerieEquipmentSlot>> Slots;

	// For clients, this is the last received hash for the serverside equipment state.
	// For servers, this is identical to LocalChecksum.
	UPROPERTY(ReplicatedUsing = "OnRep_ServerChecksum")
	FFaerieEquipmentHash ServerChecksum;

	// This is the hash of the current local equipment.
	// This is compared against ServerChecksum each time equipment is changed to verify the client has received the
	// current equipment state.
	FFaerieEquipmentHash LocalChecksum;

	// Are our checksums known to currently match.
	bool ChecksumsMatch = true;
};