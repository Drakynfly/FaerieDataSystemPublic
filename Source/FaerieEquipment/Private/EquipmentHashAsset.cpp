// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "EquipmentHashAsset.h"
#include "EquipmentHashStatics.h"
#include "UObject/ObjectSaveContext.h"
#include "Squirrel.h"

#if WITH_EDITOR
#include "FaerieItemAsset.h"
#include "FaerieItemStackHashInstruction.h"
#endif

#include UE_INLINE_GENERATED_CPP_BY_NAME(EquipmentHashAsset)

void UFaerieEquipmentHashAsset::PreSave(FObjectPreSaveContext SaveContext)
{
	Super::PreSave(SaveContext);

#if WITH_EDITOR
	CheckHash = 0;

	for (auto&& Config : Configs)
	{
		if (!IsValid(Config.Instruction))
		{
			continue;
		}

		for (int32 i = 0; i < Config.Slots.Num(); ++i)
		{
			int32 TagHash = 0;

			if (Config.Example.IsValidIndex(i) &&
				IsValid(Config.Example[i]))
			{
				TagHash = Config.Instruction->Hash({Config.Example[i]->GetEditorItemView(), 1});

				if (Config.MatchType == EGameplayContainerMatchType::Any)
				{
					CheckHash = Squirrel::HashCombine(CheckHash, TagHash);
					break;
				}
			}

			CheckHash = Squirrel::HashCombine(CheckHash, TagHash);
		}
	}
#endif
}