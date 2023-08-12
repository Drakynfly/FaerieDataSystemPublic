// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "FaerieItemStack.generated.h"

class UFaerieItem;
class IFaerieItemOwnerInterface;

/**
 * A simple stack of items.
 */
USTRUCT(BlueprintType)
struct FAERIEITEMDATA_API FFaerieItemStack
{
	GENERATED_BODY()

	FFaerieItemStack() {}

	FFaerieItemStack(UFaerieItem* ItemData, const int32 Copies)
	  : Item(ItemData),
		Copies(Copies) {}

	// The item being counted
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FaerieItemStack")
	TObjectPtr<UFaerieItem> Item;

	// Copies in this stack
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FaerieItemStack")
	int32 Copies = 0;

	friend bool operator==(const FFaerieItemStack& Lhs, const FFaerieItemStack& Rhs)
	{
		return Lhs.Item == Rhs.Item
			   && Lhs.Copies == Rhs.Copies;
	}

	friend bool operator!=(const FFaerieItemStack& Lhs, const FFaerieItemStack& Rhs)
	{
		return !(Lhs == Rhs);
	}
};

/**
 * A simple stack of items. Item reference is const
 */
USTRUCT(BlueprintType)
struct FAERIEITEMDATA_API FFaerieItemStackView
{
	GENERATED_BODY()

	FFaerieItemStackView() {}

	FFaerieItemStackView(const UFaerieItem* ItemData, const int32 Copies)
	  : Item(ItemData),
		Copies(Copies) {}

	FFaerieItemStackView(const FFaerieItemStack Stack)
	  : Item(Stack.Item),
		Copies(Stack.Copies) {}

	// The item being counted
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FaerieItemStack")
	TObjectPtr<const UFaerieItem> Item;

	// Copies in this stack
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FaerieItemStack")
	int32 Copies = 0;

	friend bool operator==(const FFaerieItemStackView Lhs, const FFaerieItemStackView Rhs)
	{
		return Lhs.Item == Rhs.Item
			   && Lhs.Copies == Rhs.Copies;
	}

	friend bool operator!=(const FFaerieItemStackView Lhs, const FFaerieItemStackView Rhs)
	{
		return !(Lhs == Rhs);
	}
};