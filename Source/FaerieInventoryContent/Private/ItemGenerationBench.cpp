// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "ItemGenerationBench.h"

#include "ItemGeneratorConfig.h"

#if WITH_EDITOR
void UItemGenerationBench::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(UItemGenerationBench, Drivers))
	{
		for (TObjectPtr<UItemGenerationDriver>& Driver : Drivers)
		{
			if (!Driver)
			{
				Driver = NewObject<UItemGenerationDriver>(this);
			}
		}
	}
}
#endif