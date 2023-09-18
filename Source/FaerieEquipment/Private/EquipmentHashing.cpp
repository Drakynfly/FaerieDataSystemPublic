// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "EquipmentHashing.h"
#include "FaerieEquipmentManager.h"
#include "FaerieEquipmentSlot.h"
#include "Squirrel.h"
#include "DelegateCommon.h"
#include "Tokens/FaerieInfoToken.h"

// WARNING: Changing this will invalidate all existing hashes generated with MakeEquipmentHash.
#define EQUIPMENT_HASHING_SEED 561333781

namespace Faerie::Hash
{
	FORCEINLINE int32 Combine(const int32 A, const int32 B)
	{
		return Squirrel::HashCombine(A, B);
	}

	int32 HashFProperty(const void* Ptr, const FProperty* Property)
	{
		if (Property->IsA<FBoolProperty>())
		{
			return GetTypeHash(*Property->ContainerPtrToValuePtr<bool>(Ptr));
		}
		if (Property->IsA<FByteProperty>() || Property->IsA<FEnumProperty>())
		{
			return GetTypeHash(*Property->ContainerPtrToValuePtr<uint8>(Ptr));
		}
		if (Property->IsA<FIntProperty>())
		{
			return GetTypeHash(*Property->ContainerPtrToValuePtr<int32>(Ptr));
		}
		if (Property->IsA<FInt64Property>())
		{
			return GetTypeHash(*Property->ContainerPtrToValuePtr<int64>(Ptr));
		}
		if (Property->IsA<FFloatProperty>())
		{
			return GetTypeHash(*Property->ContainerPtrToValuePtr<float>(Ptr));
		}
		if (Property->IsA<FDoubleProperty>())
		{
			return GetTypeHash(*Property->ContainerPtrToValuePtr<double>(Ptr));
		}
		if (Property->IsA<FNameProperty>())
		{
			return GetTypeHash(*Property->ContainerPtrToValuePtr<FName>(Ptr));
		}
		if (Property->IsA<FStrProperty>())
		{
			return GetTypeHash(*Property->ContainerPtrToValuePtr<FString>(Ptr));
		}
		if (Property->IsA<FTextProperty>())
		{
			return GetTypeHash(Property->ContainerPtrToValuePtr<FText>(Ptr)->BuildSourceString());
		}
		if (const FStructProperty* AsStruct = CastField<FStructProperty>(Property))
		{
			return HashStructByProps(AsStruct->ContainerPtrToValuePtr<void>(Ptr), AsStruct->Struct);
		}
		return 0;
	}

	int32 HashFProperty(const UObject* Obj, const FProperty* Property)
	{
		if (const FBoolProperty* AsBool = CastField<FBoolProperty>(Property))
		{
			return GetTypeHash(*AsBool->ContainerPtrToValuePtr<bool>(Obj));
		}
		return 0;
	}

	template <typename T>
	int32 HashContainerProps(const T* Container, const UStruct* Struct)
	{
		int32 Hash = 0;

		for (TFieldIterator<FProperty> PropIt(Struct, EFieldIteratorFlags::IncludeSuper); PropIt; ++PropIt)
		{
			if (const FProperty* Property = *PropIt)
			{
				Hash = Combine(HashFProperty(Container, Property), Hash);
			}
		}

		return Hash;
	}

	int32 HashStructByProps(const void* Ptr, const UScriptStruct* Struct)
	{
		check(Ptr);
		check(Struct);
		return HashContainerProps(Ptr, Struct);
	}

	int32 HashItemByProps(const UObject* Obj)
	{
		check(Obj);
		return HashContainerProps(Obj, Obj->GetClass());
	}
}

FFaerieEquipmentHash UFaerieEquipmentHashing::MakeEquipmentHash(const TConstArrayView<int32> Hashes)
{
	// return a 0 hash for an empty array.
	if (Hashes.IsEmpty()) return FFaerieEquipmentHash();

	FFaerieEquipmentHash OutHash{EQUIPMENT_HASHING_SEED};

	// Combine all hashes into the final hash
	for (const int32 Hash : Hashes)
	{
		OutHash.Hash = Faerie::Hash::Combine(Hash, OutHash.Hash);
	}

	return OutHash;
}

FFaerieEquipmentHash UFaerieEquipmentHashing::HashItemSet(const TSet<const UFaerieItem*>& Items,
														  const FEquipmentHashFunction& Function)
{
	TArray<int32> Hashes;

	for (const UFaerieItem* Item : Items)
	{
		Hashes.Add(Function(Item));
	}

	return MakeEquipmentHash(Hashes);
}

FFaerieEquipmentHash UFaerieEquipmentHashing::HashEquipment(const UFaerieEquipmentManager* Manager,
															const TSet<FFaerieSlotTag>& Slots, const FEquipmentHashFunction& Function)
{
	if (!IsValid(Manager))
	{
		return FFaerieEquipmentHash();
	}

	TArray<int32> Hashes;

	for (const FFaerieSlotTag SlotTag : Slots)
	{
		if (const UFaerieEquipmentSlot* Slot = Manager->FindSlot(SlotTag))
		{
			if (Slot->IsFilled())
			{
				Hashes.Add(Function(Slot->GetItemObject()));
			}
		}
	}

	return MakeEquipmentHash(Hashes);
}

FFaerieEquipmentHash UFaerieEquipmentHashing::HashEquipment(const UFaerieEquipmentManager* Manager,
															const FFaerieEquipmentHashConfig& Config)
{
	if (!Config.HashFunction.IsBound())
	{
		return FFaerieEquipmentHash();
	}

	return HashEquipment(Manager, Config.Slots,
		[Function = Config.HashFunction](const UFaerieItem* Item)
		{
			return Function.Execute(Item);
		});
}

FBlueprintEquipmentHash UFaerieEquipmentHashing::GetEquipmentHash_ByName()
{
	return AUTO_DELEGATE_STATIC(FBlueprintEquipmentHash, ThisClass, ExecHashEquipmentByName);
}

int32 UFaerieEquipmentHashing::ExecHashEquipmentByName(const UFaerieItem* Item)
{
	if (!IsValid(Item)) return 0;

	if (const UFaerieInfoToken* InfoToken = Item->GetToken<UFaerieInfoToken>())
	{
		return GetTypeHash(InfoToken->GetItemName().BuildSourceString());
	}

	return 0;
}

bool UFaerieEquipmentQueryLibrary::RunEquipmentQuery(const UFaerieEquipmentManager* Manager, const FFaerieEquipmentSetQuery& SetQuery, UFaerieEquipmentSlot*& PassingSlot)
{
	for (auto&& QueryTag : SetQuery.TagSet.Tags)
	{
		if (auto&& Slot = Manager->FindSlot(QueryTag))
		{
			if (Slot->IsFilled())
			{
				if (SetQuery.Query.Filter.Execute(Slot->View()) != SetQuery.Query.InvertFilter)
				{
					PassingSlot = Slot;
					return true;
				}
			}
		}
	}

	PassingSlot = nullptr;
	return false;
}