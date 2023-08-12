// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "FaerieItem.h"
#include "FaerieItemToken.h"
#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"

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
	// either duplicate or rename the object with us as the outer.
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

	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, LastModified, this);
	LastModified = FDateTime::UtcNow();

	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, Tokens, this);
	return !!Tokens.Remove(Token);
}

bool UFaerieItem::RemoveTokenByClass(const TSubclassOf<UFaerieItemToken> Class)
{
	UFaerieItemToken* Token = GetToken(Class);

	if (!IsValid(Token))
	{
		return false;
	}

	return RemoveToken(Token);
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
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, LastModified, this);
	LastModified = FDateTime::UtcNow();
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