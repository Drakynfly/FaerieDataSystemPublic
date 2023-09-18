// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "EquipmentHashTypes.h"
#include "Components/ActorComponent.h"
#include "FaerieSlotTag.h"

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

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEquipmentClientChecksumEvent, EFaerieEquipmentClientChecksumState, State);

UCLASS(ClassGroup = ("Faerie"), meta = (BlueprintSpawnableComponent),
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
	virtual void ReadyForReplication() override;
	//~ UActorComponent

protected:
	void OnSlotItemChanged(UFaerieEquipmentSlot* FaerieEquipmentSlot);

	void RecalcLocalChecksum();

	void CheckLocalChecksum();

	UFUNCTION()
	void OnRep_ServerChecksum();

public:
	/**------------------------------*/
	/*			SLOTS API			 */
	/**------------------------------*/

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Faerie|EquipmentManager")
	UFaerieEquipmentSlot* AddSlot(FFaerieSlotTag SlotID, UFaerieEquipmentSlotDescription* Description);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Faerie|EquipmentManager")
	bool RemoveSlot(UFaerieEquipmentSlot* Slot);

	UFUNCTION(BlueprintCallable, Category = "Faerie|EquipmentManager")
	TArray<UFaerieEquipmentSlot*> GetSlots() const { return Slots; }

	UFUNCTION(BlueprintCallable, Category = "Faerie|EquipmentManager")
	UFaerieEquipmentSlot* FindSlot(FFaerieSlotTag SlotID) const;


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
public:
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FEquipmentClientChecksumEvent OnClientChecksumEvent;

protected:
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