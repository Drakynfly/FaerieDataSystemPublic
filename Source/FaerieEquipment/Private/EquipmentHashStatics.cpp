// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "EquipmentHashStatics.h"
#include "EquipmentHashAsset.h"
#include "FaerieEquipmentManager.h"
#include "FaerieEquipmentSlot.h"
#include "FaerieItemStackHashInstruction.h"

namespace Faerie::Hash
{
	FFaerieHash HashEquipment(const UFaerieEquipmentManager* Manager,
							  const TSet<FFaerieSlotTag>& Slots, const FItemHashFunction& Function)
	{
		if (!IsValid(Manager))
		{
			return FFaerieHash();
		}

		TArray<uint32> Hashes;
		Hashes.Reserve(Slots.Num());

		for (const FFaerieSlotTag SlotTag : Slots)
		{
			if (const UFaerieEquipmentSlot* Slot = Manager->FindSlot(SlotTag, true))
			{
				Hashes.Add(Function(Slot->GetItemObject()));
			}
		}

		return CombineHashes(Hashes);
	}

	bool ExecuteHashInstructions(const UFaerieEquipmentManager* Manager, const UFaerieEquipmentHashAsset* Asset)
	{
		if (!IsValid(Manager) || !IsValid(Asset))
		{
			return false;
		}

		uint32 FinalHash = 0;

		for (auto&& Config : Asset->Configs)
		{
			for (const FGameplayTag Tag : Config.Slots)
			{
				const FFaerieSlotTag SlotTag = FFaerieSlotTag::ConvertChecked(Tag);

				uint32 TagHash = 0;

				if (auto&& Slot = Manager->FindSlot(SlotTag, true))
				{
					if (Slot->IsFilled())
					{
						TagHash = Config.Instruction->Hash(Slot->View());

						if (Config.MatchType == EGameplayContainerMatchType::Any)
						{
							FinalHash = Combine(FinalHash, TagHash);
							break;
						}
					}
				}

				FinalHash = Combine(FinalHash, TagHash);
			}
		}

		return FinalHash == Asset->CheckHash;
	}
}