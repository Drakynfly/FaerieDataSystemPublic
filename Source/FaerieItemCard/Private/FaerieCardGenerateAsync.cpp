// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "FaerieCardGenerateAsync.h"
#include "FaerieCardGenerator.h"
#include "FaerieCardGeneratorInterface.h"
#include "FaerieItemDataProxy.h"

UFaerieCardGenerateAsync::UFaerieCardGenerateAsync(const FObjectInitializer& ObjectInitializer)
{
}

UFaerieCardGenerateAsync* UFaerieCardGenerateAsync::GenerateFaerieCardAsync(const TScriptInterface<IFaerieCardGeneratorInterface> Generator,
	UFaerieItemDataProxyBase* Proxy, const EFaerieCardGeneratorType Type)
{
	UFaerieCardGenerateAsync* AsyncAction = NewObject<UFaerieCardGenerateAsync>();
	AsyncAction->Generator = Generator;
	AsyncAction->Proxy = Proxy;
	AsyncAction->Type = Type;
	return AsyncAction;
}

void UFaerieCardGenerateAsync::Activate()
{
	Super::Activate();

	if (IsValid(Generator.GetObject()) && IsValid(Proxy))
	{
		Generator->GetGenerator()->Generate(Proxy, Type, FFaerieCardGenerationResult::CreateUObject(this, &ThisClass::OnCardGenerationFinished));
	}
}

void UFaerieCardGenerateAsync::OnCardGenerationFinished(const bool Success, UFaerieCardBase* Widget)
{
	if (Success)
	{
		OnSuccess.Broadcast(Widget);
	}
	else
	{
		OnFailure.Broadcast();
	}
}
