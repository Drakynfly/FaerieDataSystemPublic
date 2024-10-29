// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "FaerieItemDataLibrary.h"
#include "FaerieItem.h"
#include "FaerieItemToken.h"

bool UFaerieItemDataLibrary::Equal_ItemData(const UFaerieItem* A, const UFaerieItem* B)
{
	// If they are the same object, then we already know they are identical
	if (A == B)
	{
		return true;
	}

	// If either is mutable then they are considered "unequivocable" and therefor, mutually exclusive.
	if (A->IsDataMutable() || B->IsDataMutable())
	{
		return false;
	}

	// Resort to comparing tokens ...
	const TConstArrayView<TObjectPtr<UFaerieItemToken>> TokensA = A->GetTokens();
	const TConstArrayView<TObjectPtr<UFaerieItemToken>> TokensB = B->GetTokens();

	// This already indicates they are not equal.
	if (TokensA.Num() != TokensB.Num())
	{
		return false;
	}

	TMap<UClass*, TObjectPtr<UFaerieItemToken>> TokenMapA;

	// Get the classes of tokens in A
	for (auto&& Token : TokensA)
	{
		TokenMapA.Add(Token.GetClass(), Token);
	}

	TArray<TPair<TObjectPtr<UFaerieItemToken>, TObjectPtr<UFaerieItemToken>>> TokenPairs;

	// Check that for every token in A, there is one that matches class in B
	for (auto&& TokenB : TokensB)
	{
		auto&& TokenA = TokenMapA.Find(TokenB.GetClass());

		if (TokenA == nullptr)
		{
			return false;
		}

		TokenPairs.Add({*TokenA, TokenB});
	}

	for (auto&& [AToken, BToken] : TokenPairs)
	{
		if (!AToken->CompareWith(BToken))
		{
			return false;
		}
	}

	// They are equal then :)
	return true;
}

bool UFaerieItemDataLibrary::Equal_ItemToken(const UFaerieItemToken* A, const UFaerieItemToken* B)
{
	return A->CompareWith(B);
}