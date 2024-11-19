// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "Components/ActorComponent.h"
#include "UObject/WeakInterfacePtr.h"
#include "FaerieItemDataProxy.h"

#include "EquipmentVisualizer.generated.h"

USTRUCT(BlueprintType)
struct FFaerieVisualKey
{
	GENERATED_BODY()

	FFaerieItemProxy Proxy;

	bool IsValid() const
	{
		return Proxy.IsValid();
	}

	friend bool operator==(const FFaerieVisualKey& Lhs, const FFaerieVisualKey& Rhs) { return Lhs.Proxy == Rhs.Proxy; }
	friend bool operator!=(const FFaerieVisualKey& Lhs, const FFaerieVisualKey& Rhs) { return !(Lhs == Rhs); }

	friend uint32 GetTypeHash(const FFaerieVisualKey& VisualKey)
	{
		return GetTypeHash(VisualKey.Proxy.GetObject());
	}
};

USTRUCT(Blueprintable)
struct FEquipmentVisualAttachment
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EquipmentVisualAttachment")
	TWeakObjectPtr<USceneComponent> Parent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EquipmentVisualAttachment")
	FName Socket;

	FAttachmentTransformRules TransformRules = FAttachmentTransformRules::SnapToTargetNotIncludingScale;
};

DECLARE_DYNAMIC_DELEGATE_TwoParams(FEquipmentVisualizerCallback, FFaerieVisualKey, Key, UObject*, Visual);

USTRUCT(Blueprintable)
struct FEquipmentVisualMetadata
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EquipmentVisualMetadata")
	FEquipmentVisualAttachment Attachment;

	FEquipmentVisualizerCallback ChangeCallback;
};

using FEquipmentVisualizerUpdateNative = TMulticastDelegate<void(FFaerieVisualKey, UObject*)>;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEquipmentVisualizerUpdate, FFaerieVisualKey, Key, UObject*, Visual);


/**
 * An actor component that spawns visuals for equipment from a linked Equipment Manager
 */
UCLASS(ClassGroup = ("Faerie"), meta=(BlueprintSpawnableComponent))
class FAERIEEQUIPMENT_API UEquipmentVisualizer : public UActorComponent
{
	GENERATED_BODY()

public:
	UEquipmentVisualizer();

	//~ UActorComponent
	virtual void OnComponentDestroyed(bool bDestroyingHierarchy) override;
	//~ UActorComponent

	FEquipmentVisualizerUpdateNative::RegistrationType& GetOnAnyVisualUpdate() { return OnAnyVisualUpdateNative; }

	UFUNCTION(BlueprintCallable, Category = "Faerie|EquipmentVisualizer", meta = (DeterminesOutputType = "Class"))
	UObject* GetSpawnedVisualByClass(TSubclassOf<UObject> Class, FFaerieVisualKey& Key) const;

	UFUNCTION(BlueprintCallable, Category = "Faerie|EquipmentVisualizer", meta = (DeterminesOutputType = "Class"))
	AActor* GetSpawnedActorByClass(TSubclassOf<AActor> Class, FFaerieVisualKey& Key) const;

	UFUNCTION(BlueprintCallable, Category = "Faerie|EquipmentVisualizer", meta = (DeterminesOutputType = "Class"))
	USceneComponent* GetSpawnedComponentByClass(TSubclassOf<USceneComponent> Class, FFaerieVisualKey& Key) const;

	UFUNCTION(BlueprintCallable, Category = "Faerie|EquipmentVisualizer")
	UObject* GetSpawnedVisualByKey(FFaerieVisualKey Key) const;

	UFUNCTION(BlueprintCallable, Category = "Faerie|EquipmentVisualizer")
	AActor* GetSpawnedActorByKey(FFaerieVisualKey Key) const;

	UFUNCTION(BlueprintCallable, Category = "Faerie|EquipmentVisualizer")
	USceneComponent* GetSpawnedComponentByKey(FFaerieVisualKey Key) const;

	template <typename TActor>
	TActor* SpawnVisualActorNative(FFaerieVisualKey Key, TSubclassOf<TActor> Class, const FEquipmentVisualAttachment& Attachment)
	{
		return Cast<TActor>(SpawnVisualActor(Key, Class, Attachment));
	}

	UFUNCTION(BlueprintCallable, Category = "Faerie|EquipmentVisualizer", meta = (AutoCreateRefTerm = "Attachment", DeterminesOutputType = "Class"))
	AActor* SpawnVisualActor(FFaerieVisualKey Key, TSubclassOf<AActor> Class, const FEquipmentVisualAttachment& Attachment);

	UFUNCTION(BlueprintCallable, Category = "Faerie|EquipmentVisualizer")
	bool DestroyVisualActor(FFaerieVisualKey Key, bool ClearMetadata = false);

	UFUNCTION(BlueprintCallable, Category = "Faerie|EquipmentVisualizer")
	void ResetAttachment(FFaerieVisualKey Key);

	UFUNCTION(BlueprintCallable, Category = "Faerie|EquipmentVisualizer")
	void AwaitOrReceiveUpdate(FFaerieVisualKey Key, FEquipmentVisualizerCallback Callback);

	UFUNCTION(BlueprintPure)
	static FFaerieVisualKey MakeVisualKeyFromProxy(const TScriptInterface<IFaerieItemDataProxy>& Proxy);

protected:
	UFUNCTION(/* Dynamic Callback */)
	virtual void OnVisualActorDestroyed(AActor* DestroyedActor);

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FEquipmentVisualizerUpdate OnAnyVisualUpdate;

protected:
	UPROPERTY()
	TMap<FFaerieVisualKey, TObjectPtr<AActor>> SpawnedActors;

	UPROPERTY()
	TMap<TObjectPtr<AActor>, FFaerieVisualKey> ReverseMap;

	UPROPERTY()
	TMap<FFaerieVisualKey, TObjectPtr<USceneComponent>> SpawnedComponents;

	UPROPERTY()
	TMap<FFaerieVisualKey, FEquipmentVisualMetadata> KeyedMetadata;

private:
	FEquipmentVisualizerUpdateNative OnAnyVisualUpdateNative;
};