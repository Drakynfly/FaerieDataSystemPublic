// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "FaerieCardGenerator.h"
#include "CardTokens/CustomCardClass.h"

#include "FaerieItemDataProxy.h"
#include "FaerieItem.h"

// @todo temp
#include "FaerieCardSubsystem.h"


void UFaerieCardGenerator::Generate(const UFaerieItemDataProxyBase* ItemData, const EFaerieCardGeneratorType Type,
                                    const FFaerieCardGenerationResultDynamic& Callback)
{
	FFaerieCardGenerationResult NativeCallback;

	if (Callback.IsBound())
	{
		NativeCallback.BindWeakLambda(Callback.GetUObject(), [Callback](const bool Success, UFaerieCardBase* Widget)
		{
			Callback.Execute(Success, Widget);
		});
	}

	Generate(ItemData, Type, NativeCallback);
}

void UFaerieCardGenerator::Generate(const UFaerieItemDataProxyBase* ItemData, const EFaerieCardGeneratorType Type, const FFaerieCardGenerationResult& Callback)
{
	if (!IsValid(ItemData))
	{
		Callback.Execute(false, nullptr);
		return;
	}

	auto&& Item = ItemData->GetItemObject();

	if (!IsValid(Item))
	{
		Callback.Execute(false, nullptr);
		return;
	}

	const UCustomCardClass* CardClassProvider = nullptr;

	switch (Type)
	{
	case EFaerieCardGeneratorType::Palette:
		CardClassProvider = Item->GetToken<UCustomPaletteCard>();
		break;
	case EFaerieCardGeneratorType::Info:
		CardClassProvider = Item->GetToken<UCustomInfoCard>();
		break;
	default: ;
	}

	TSoftClassPtr<UFaerieCardBase> CardClass;

	if (CardClassProvider)
	{
		CardClass = CardClassProvider->GetCardClass();
	}
	else
	{
		switch (Type)
		{
		case EFaerieCardGeneratorType::Palette:
			CardClass = DefaultPaletteClass;
			break;
		case EFaerieCardGeneratorType::Info:
			CardClass = DefaultInfoClass;
			break;
		default: ;
		}
	}

	if (!CardClass.IsNull())
	{
		StreamableManager.RequestAsyncLoad(CardClass.ToSoftObjectPath(),
			FStreamableDelegate::CreateUObject(this, &ThisClass::OnCardClassLoaded, CardClass, ItemData, Callback));
	}
	else
	{
		Callback.Execute(false, nullptr);
	}
}

void UFaerieCardGenerator::OnCardClassLoaded(const TSoftClassPtr<UFaerieCardBase> Class, const UFaerieItemDataProxyBase* ItemData, FFaerieCardGenerationResult Callback)
{
	const TSubclassOf<UFaerieCardBase> LoadedClass = Class.Get();

	if (IsValid(LoadedClass))
	{
		auto&& CardSS = GetTypedOuter<UFaerieCardSubsystem>();
		auto&& Player = CardSS->GetLocalPlayer<ULocalPlayer>()->PlayerController;

		UFaerieCardBase* CardWidget = CreateWidget<UFaerieCardBase>(Player, LoadedClass);

		if (IsValid(CardWidget))
		{
			CardWidget->SetItemData(ItemData, false);
		}

		Callback.ExecuteIfBound(IsValid(CardWidget), CardWidget);
	}
	else
	{
		Callback.ExecuteIfBound(false, nullptr);
	}
}