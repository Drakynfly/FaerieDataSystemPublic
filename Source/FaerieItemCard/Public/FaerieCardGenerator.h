// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "UObject/Object.h"
#include "Engine/StreamableManager.h"

#include "Widgets/FaerieCardBase.h"

#include "FaerieCardGenerator.generated.h"

UENUM(BlueprintType)
enum class EFaerieCardGeneratorType : uint8
{
	Palette,
	Info
};

using FFaerieCardGenerationResult = TDelegate<void(bool, UFaerieCardBase*)>;
DECLARE_DYNAMIC_DELEGATE_TwoParams(FFaerieCardGenerationResultDynamic, bool, Success, UFaerieCardBase*, Widget);

/**
 *
 */
UCLASS()
class FAERIEITEMCARD_API UFaerieCardGenerator : public UObject
{
	GENERATED_BODY()

	friend class UFaerieCardSubsystem;

public:
	void Generate(const UFaerieItemDataProxyBase* ItemData, EFaerieCardGeneratorType Type, const FFaerieCardGenerationResultDynamic& Callback);
	void Generate(const UFaerieItemDataProxyBase* ItemData, EFaerieCardGeneratorType Type, const FFaerieCardGenerationResult& Callback);

protected:
	void OnCardClassLoaded(TSoftClassPtr<UFaerieCardBase> Class, const UFaerieItemDataProxyBase* ItemData, FFaerieCardGenerationResult Callback);

protected:
	UPROPERTY()
	TSoftClassPtr<UFaerieCardBase> DefaultPaletteClass;

	UPROPERTY()
	TSoftClassPtr<UFaerieCardBase> DefaultInfoClass;

private:
	FStreamableManager StreamableManager;
};