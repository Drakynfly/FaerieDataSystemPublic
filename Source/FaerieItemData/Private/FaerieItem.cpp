﻿// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "FaerieItem.h"
#include "FaerieItemToken.h"
#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"
#include "UObject/ObjectSaveContext.h"

#if WITH_EDITOR
// This is really the module startup time, since this is set whenever this module loads :)
static FDateTime EditorStartupTime = FDateTime::UtcNow();
#endif

void UFaerieItem::PreSave(FObjectPreSaveContext SaveContext)
{
	CacheTokenMutability();

#if WITH_EDITOR
	// This is a random hoot I'm adding to be funny. The LastModified timestamp only really matters for mutable items,
	// so for any other random item in the game, this time would be meaningless. So instead I'm going to encode the time
	// that the item was last saved in-editor. For items visible in-editor this is will tell the dev when the item was
	// last touched. Since PreSave is called on all assets during packaging, theoretically, assets in a packaged build
	// will all have the same timestamp.
	LastModified = EditorStartupTime;
#endif

	Super::PreSave(SaveContext);
}

void UFaerieItem::PostLoad()
{
	Super::PostLoad();
	CacheTokenMutability();
}

void UFaerieItem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams SharedParams;
	SharedParams.bIsPushBased = true;
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, Tokens, SharedParams);
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, LastModified, SharedParams);
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, MutabilityFlags, SharedParams);
}

void UFaerieItem::ForEachToken(const TFunction<bool(const UFaerieItemToken*)>& Iter) const
{
	for (auto&& Token : Tokens)
	{
		if (IsValid(Token))
		{
			if (!Iter(Token))
			{
				return;
			}
		}
	}
}

void UFaerieItem::ForEachTokenOfClass(const TFunction<bool(const UFaerieItemToken*)>& Iter, const TSubclassOf<UFaerieItemToken> Class) const
{
	for (auto&& Token : Tokens)
	{
		if (IsValid(Token) && Token->IsA(Class))
		{
			if (!Iter(Token))
			{
				return;
			}
		}
	}
}

UFaerieItem* UFaerieItem::CreateInstance()
{
	UFaerieItem* Instance = NewObject<UFaerieItem>(GetTransientPackage());
	EnumAddFlags(Instance->MutabilityFlags, EFaerieItemMutabilityFlags::InstanceMutability);
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, MutabilityFlags, Instance);
	Instance->LastModified = FDateTime::UtcNow();
	return Instance;
}

UFaerieItem* UFaerieItem::CreateDuplicate() const
{
	UFaerieItem* Duplicate = DuplicateObject(this, GetTransientPackage());
	EnumAddFlags(Duplicate->MutabilityFlags, EFaerieItemMutabilityFlags::InstanceMutability);
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, MutabilityFlags, Duplicate);
	Duplicate->LastModified = FDateTime::UtcNow();
	return Duplicate;
}

UFaerieItemToken* UFaerieItem::GetTokenImpl(const TSubclassOf<UFaerieItemToken> Class)
{
	if (!ensure(IsValid(Class)))
	{
		return nullptr;
	}

	if (!ensure(Class != UFaerieItemToken::StaticClass()))
	{
		return nullptr;
	}

	// If we are not DataMutable, don't allow non-const access to tokens.
	if (!IsDataMutable())
	{
		return nullptr;
	}

	for (auto&& Token : Tokens)
	{
		if (IsValid(Token) && Token.IsA(Class))
		{
			return Token;
		}
	}

	return nullptr;
}

const UFaerieItemToken* UFaerieItem::GetTokenImpl(const TSubclassOf<UFaerieItemToken> Class) const
{
	if (!ensure(IsValid(Class)))
	{
		return nullptr;
	}

	if (!ensure(Class != UFaerieItemToken::StaticClass()))
	{
		return nullptr;
	}

	for (auto&& Token : Tokens)
	{
		if (IsValid(Token) && Token.IsA(Class))
		{
			return Token;
		}
	}

	return nullptr;
}

UFaerieItemToken* UFaerieItem::GetToken(const TSubclassOf<UFaerieItemToken> Class) const
{
	if (!ensure(IsValid(Class)))
	{
		return nullptr;
	}

	if (!ensure(Class != UFaerieItemToken::StaticClass()))
	{
		return nullptr;
	}

	for (auto&& Token : Tokens)
	{
		if (IsValid(Token) && Token.IsA(Class))
		{
			return Token;
		}
	}

	return nullptr;
}

bool UFaerieItem::FindToken(const TSubclassOf<UFaerieItemToken> Class, UFaerieItemToken*& FoundToken) const
{
	if (!IsValid(Class))
	{
		return false;
	}
	FoundToken = GetToken(Class);
	return FoundToken != nullptr;
}

void UFaerieItem::FindTokens(const TSubclassOf<UFaerieItemToken> Class, TArray<UFaerieItemToken*>& FoundTokens) const
{
	if (!ensure(IsValid(Class)))
	{
		return;
	}

	if (!ensure(Class != UFaerieItemToken::StaticClass()))
	{
		return;
	}

	FoundTokens.Empty();

	Algo::CopyIf(Tokens, FoundTokens,
		[Class](const UFaerieItemToken* Token)
		{
			return IsValid(Token) && Token->IsA(Class);
		});
}

void UFaerieItem::AddToken(UFaerieItemToken* Token)
{
	if (!ensure(IsValid(Token)))
	{
		return;
	}

	if (!ensure(IsInstanceMutable()))
	{
		return;
	}

	// If this check fails, then whatever code tried to add the token didn't create it with us as the outer, or needs to
	// either duplicate or rename the token with us as the outer.
	check(Token->GetOuter() == this);

	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, LastModified, this);
	LastModified = FDateTime::UtcNow();

	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, Tokens, this);
	Tokens.Add(Token);

	CacheTokenMutability();
}

bool UFaerieItem::RemoveToken(UFaerieItemToken* Token)
{
	if (!ensure(IsValid(Token)))
	{
		return false;
	}

	if (!ensure(IsInstanceMutable()))
	{
		return false;
	}

	if (!!Tokens.Remove(Token))
	{
		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, Tokens, this);

		LastModified = FDateTime::UtcNow();
		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, LastModified, this);

		return true;
	}

	return false;
}

int32 UFaerieItem::RemoveTokensByClass(const TSubclassOf<UFaerieItemToken> Class)
{
	if (!ensure(IsValid(Class)))
	{
		return 0;
	}

	if (!ensure(Class != UFaerieItemToken::StaticClass()))
	{
		return 0;
	}

	if (!ensure(IsInstanceMutable()))
	{
		return false;
	}

	if (const int32 Removed = Tokens.RemoveAll(
		[Class](const UFaerieItemToken* Token)
		{
			return IsValid(Token) && Token->GetClass() == Class;
		}))
	{
		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, Tokens, this);

		LastModified = FDateTime::UtcNow();
		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, LastModified, this);

		return Removed;
	}

	return 0;
}

bool UFaerieItem::IsInstanceMutable() const
{
	return EnumHasAllFlags(MutabilityFlags, EFaerieItemMutabilityFlags::InstanceMutability);
}

bool UFaerieItem::IsDataMutable() const
{
	return EnumHasAllFlags(MutabilityFlags, EFaerieItemMutabilityFlags::TokenMutability);
}

void UFaerieItem::OnTokenEdited(UFaerieItemToken* Token)
{
	check(IsDataMutable())
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, LastModified, this);
	LastModified = FDateTime::UtcNow();
	NotifyOwnerOfSelfMutation.ExecuteIfBound(this, Token);
}

void UFaerieItem::CacheTokenMutability()
{
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, MutabilityFlags, this);

	for (auto&& Token : Tokens)
	{
		if (Token && Token->IsMutable())
		{
			EnumAddFlags(MutabilityFlags, EFaerieItemMutabilityFlags::TokenMutability);
			return;
		}
	}

	EnumRemoveFlags(MutabilityFlags, EFaerieItemMutabilityFlags::TokenMutability);
}