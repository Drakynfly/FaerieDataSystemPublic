// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "BasicItemDataFilters.h"
#include "Tokens/FaerieStackLimiterToken.h"

#if WITH_EDITOR
EItemDataMutabilityStatus UFilterRule_LogicalOr::GetMutabilityStatus() const
{
	for (auto&& Rule : Rules)
	{
		if (!Rule) continue;

		switch (Rule->GetMutabilityStatus())
		{
		case EItemDataMutabilityStatus::Unknown: break;
		case EItemDataMutabilityStatus::KnownMutable: return EItemDataMutabilityStatus::KnownMutable;
		case EItemDataMutabilityStatus::KnownImmutable: return EItemDataMutabilityStatus::KnownMutable;
		default: ;
		}
	}

	return Super::GetMutabilityStatus();
}
#endif

bool UFilterRule_LogicalOr::Exec(const UFaerieItemDataProxyBase* Proxy) const
{
	for (auto&& Rule : Rules)
	{
		if (Rule->Exec(Proxy))
		{
			return true;
		}
	}

	return false;
}

#if WITH_EDITOR
EItemDataMutabilityStatus UFilterRule_LogicalAnd::GetMutabilityStatus() const
{
	for (auto&& Rule : Rules)
	{
		if (!Rule) continue;

		switch (Rule->GetMutabilityStatus())
		{
		case EItemDataMutabilityStatus::Unknown: break;
		case EItemDataMutabilityStatus::KnownMutable: return EItemDataMutabilityStatus::KnownMutable;
		case EItemDataMutabilityStatus::KnownImmutable: return EItemDataMutabilityStatus::KnownMutable;
		default: ;
		}
	}

	return Super::GetMutabilityStatus();
}
#endif

bool UFilterRule_LogicalAnd::Exec(const UFaerieItemDataProxyBase* Proxy) const
{
	for (auto&& Rule : Rules)
	{
		if (!Rule->Exec(Proxy))
		{
			return false;
		}
	}

	return true;
}

EItemDataMutabilityStatus UFilterRule_Condition::GetMutabilityStatus() const
{
	return Super::GetMutabilityStatus();
}

bool UFilterRule_Condition::Exec(const UFaerieItemDataProxyBase* Proxy) const
{
	if (!ConditionRule) return false;
	if (ConditionRule->Exec(Proxy))
	{
		if (!TrueBranch) return false;
		return TrueBranch->Exec(Proxy);
	}
	return FalseBranch;
}

#if WITH_EDITOR
EItemDataMutabilityStatus UFilterRule_Ternary::GetMutabilityStatus() const
{
	// @TODO
	return Super::GetMutabilityStatus();
}
#endif

bool UFilterRule_Ternary::Exec(const UFaerieItemDataProxyBase* Proxy) const
{
	if (!ConditionRule) return false;
	if (ConditionRule->Exec(Proxy))
	{
		if (!TrueBranch) return false;
		return TrueBranch->Exec(Proxy);
	}
	if (!FalseBranch) return false;
	return FalseBranch->Exec(Proxy);
}

#if WITH_EDITOR
EItemDataMutabilityStatus UFilterRule_LogicalNot::GetMutabilityStatus() const
{
	if (InvertedRule)
	{
		return InvertedRule->GetMutabilityStatus();
	}
	return Super::GetMutabilityStatus();
}
#endif

bool UFilterRule_LogicalNot::Exec(const UFaerieItemDataProxyBase* Proxy) const
{
	return !InvertedRule->Exec(Proxy);
}

#if WITH_EDITOR
EItemDataMutabilityStatus UFilterRule_Mutability::GetMutabilityStatus() const
{
	return RequireMutable ? EItemDataMutabilityStatus::KnownMutable : EItemDataMutabilityStatus::KnownImmutable;
}
#endif

bool UFilterRule_Mutability::Exec(const UFaerieItemDataProxyBase* Proxy) const
{
	return Proxy->GetItemObject()->IsDataMutable() == RequireMutable;
}

#if WITH_EDITOR
EItemDataMutabilityStatus UFilterRule_MatchTemplate::GetMutabilityStatus() const
{
	if (IsValid(Template))
	{
		return Template->GetPattern()->GetMutabilityStatus();
	}
	return Super::GetMutabilityStatus();
}
#endif

bool UFilterRule_MatchTemplate::Exec(const UFaerieItemDataProxyBase* Proxy) const
{
	if (IsValid(Template))
	{
		return Template->TryMatch(Proxy);
	}
	return false;
}

#if WITH_EDITOR
EItemDataMutabilityStatus UFilterRule_HasTokens::GetMutabilityStatus() const
{
	for (auto&& TokenClass : TokenClasses)
	{
		if (GetDefault<UFaerieItemToken>(TokenClass)->IsMutable())
		{
			return EItemDataMutabilityStatus::KnownMutable;
		}
	}

	return Super::GetMutabilityStatus();
}
#endif

bool UFilterRule_HasTokens::Exec(const UFaerieItemDataProxyBase* Proxy) const
{
	auto TokenClassesCopy = TokenClasses;
	auto&& Tokens = Proxy->GetItemObject()->GetTokens();

	for (auto&& Token : Tokens)
	{
		if (!IsValid(Token)) continue;

		TokenClassesCopy.RemoveAllSwap([Token](const TSubclassOf<UFaerieItemToken> TokenClass)
			{
				return Token->IsA(TokenClass);
			});
	}

	return TokenClassesCopy.IsEmpty();
}

#if WITH_EDITOR
EItemDataMutabilityStatus UFilterRule_Copies::GetMutabilityStatus() const
{
	auto&& Default = Super::GetMutabilityStatus();

	// In cases where only stacks are allowed through, we know that we are only passing immutable data.

	switch (Operator) {
	case ECopiesCompareOperator::Less:				return Default;
	case ECopiesCompareOperator::LessOrEqual:		return Default;
	case ECopiesCompareOperator::Greater:			return EItemDataMutabilityStatus::KnownImmutable;
	case ECopiesCompareOperator::GreaterOrEqual:	return AmountToCompare > 1 ? EItemDataMutabilityStatus::KnownImmutable : Default;
	case ECopiesCompareOperator::Equal:				return AmountToCompare > 1 ? EItemDataMutabilityStatus::KnownImmutable : Default;
	case ECopiesCompareOperator::NotEqual:			return AmountToCompare == 1 ? EItemDataMutabilityStatus::KnownImmutable : Default;
	default: return Default;
	}
}
#endif

bool UFilterRule_Copies::Exec(const UFaerieItemDataProxyBase* Proxy) const
{
	switch (Operator) {
	case ECopiesCompareOperator::Less:				return Proxy->GetCopies() < AmountToCompare;
	case ECopiesCompareOperator::LessOrEqual:		return Proxy->GetCopies() <= AmountToCompare;
	case ECopiesCompareOperator::Greater:			return Proxy->GetCopies() > AmountToCompare;
	case ECopiesCompareOperator::GreaterOrEqual:	return Proxy->GetCopies() >= AmountToCompare;
	case ECopiesCompareOperator::Equal:				return Proxy->GetCopies() == AmountToCompare;
	case ECopiesCompareOperator::NotEqual:			return Proxy->GetCopies() != AmountToCompare;
	default: return false;
	}
}

#if WITH_EDITOR
EItemDataMutabilityStatus UFilterRule_StackLimit::GetMutabilityStatus() const
{
	auto&& Default = Super::GetMutabilityStatus();

	// In cases where only stacks are allowed through, we know that we are only passing immutable data.

	switch (Operator) {
	case EStackCompareOperator::Less:			return Default;
	case EStackCompareOperator::LessOrEqual:	return Default;
	case EStackCompareOperator::Greater:		return EItemDataMutabilityStatus::KnownImmutable;
	case EStackCompareOperator::GreaterOrEqual:	return AmountToCompare > 1 ? EItemDataMutabilityStatus::KnownImmutable : Default;
	case EStackCompareOperator::Equal:			return AmountToCompare > 1 ? EItemDataMutabilityStatus::KnownImmutable : Default;
	case EStackCompareOperator::NotEqual:		return AmountToCompare == 1 ? EItemDataMutabilityStatus::KnownImmutable : Default;
	case EStackCompareOperator::HasLimit:		return Default;
	case EStackCompareOperator::HasNoLimit:		return EItemDataMutabilityStatus::KnownImmutable;;
	default: return Default;
	}
}
#endif

bool UFilterRule_StackLimit::Exec(const UFaerieItemDataProxyBase* Proxy) const
{
	const FInventoryStack Limit = UFaerieStackLimiterToken::GetItemStackLimit(Proxy->GetItemObject());

	if (Limit == FInventoryStack::UnlimitedStack)
	{
		switch (Operator) {
		case EStackCompareOperator::Less:			return false;
		case EStackCompareOperator::LessOrEqual:	return false;
		case EStackCompareOperator::Greater:		return true;
		case EStackCompareOperator::GreaterOrEqual:	return true;
		case EStackCompareOperator::Equal:			return false;
		case EStackCompareOperator::NotEqual:		return true;
		case EStackCompareOperator::HasLimit:		return false;
		case EStackCompareOperator::HasNoLimit:		return true;
		default: return false;
		}
	}
	else
	{
		switch (Operator) {
		case EStackCompareOperator::Less:			return Limit < AmountToCompare;
		case EStackCompareOperator::LessOrEqual:	return Limit <= AmountToCompare;
		case EStackCompareOperator::Greater:		return Limit > AmountToCompare;
		case EStackCompareOperator::GreaterOrEqual:	return Limit >= AmountToCompare;
		case EStackCompareOperator::Equal:			return Limit == AmountToCompare;
		case EStackCompareOperator::NotEqual:		return Limit != AmountToCompare;
		case EStackCompareOperator::HasLimit:		return true;
		case EStackCompareOperator::HasNoLimit:		return false;
		default: return false;
		}
	}
}