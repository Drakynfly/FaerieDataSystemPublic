// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#pragma once

#include "FaerieItemStackHashInstruction.h"
#include "BasicItemHashInstructions.generated.h"

class UFaerieItemToken;
class UFaerieItemDataFilter;


/**
 * Inject a manually defined hash.
 */
UCLASS(DisplayName = "Literal (FISHI)")
class UFISHI_Literial : public UFaerieItemStackHashInstruction
{
	GENERATED_BODY()

public:
	virtual int32 Hash(FFaerieItemStackView StackView) const override;

protected:
	UPROPERTY(EditAnywhere, Category = "FISHI")
	int32 Value = 0;
};

/**
 * Inject a manually defined hash.
 */
UCLASS(DisplayName = "Validate (FISHI)")
class UFISHI_IsValid : public UFaerieItemStackHashInstruction
{
	GENERATED_BODY()

public:
	virtual int32 Hash(FFaerieItemStackView StackView) const override;
};


/**
 * Performs a Hash Combine of multiple instructions.
 */
UCLASS(DisplayName = "And (FISHI)")
class UFISHI_And : public UFaerieItemStackHashInstruction
{
	GENERATED_BODY()

public:
	virtual int32 Hash(FFaerieItemStackView StackView) const override;

protected:
	UPROPERTY(EditAnywhere, Instanced, Category = "FISHI")
	TArray<TObjectPtr<UFaerieItemStackHashInstruction>> Instructions;
};


/**
 * Returns the hash for the first instruction to not fail
 */
UCLASS(DisplayName = "Or (FISHI)")
class UFISHI_Or : public UFaerieItemStackHashInstruction
{
	GENERATED_BODY()

public:
	virtual int32 Hash(FFaerieItemStackView StackView) const override;

protected:
	UPROPERTY(EditAnywhere, Instanced, Category = "FISHI")
	TArray<TObjectPtr<UFaerieItemStackHashInstruction>> Instructions;
};


/**
 * Match against an ItemDataFilter, returning one of two predefined hashes.
 */
UCLASS(DisplayName = "Boolean Filter (FISHI)")
class UFISHI_BooleanFilter : public UFaerieItemStackHashInstruction
{
	GENERATED_BODY()

public:
	virtual int32 Hash(FFaerieItemStackView StackView) const override;

protected:
	// Pattern used to determine if an item qualifies as fitting this template.
	UPROPERTY(EditInstanceOnly, Category = "FISHI", meta = (DisplayThumbnail = false))
	TObjectPtr<UFaerieItemDataFilter> Pattern;
};

/**
 * Select between two other instructions to use, based on the result of an ItemDataFilter.
 */
UCLASS(DisplayName = "Boolean Select (FISHI)")
class UFISHI_BooleanSelect : public UFaerieItemStackHashInstruction
{
	GENERATED_BODY()

public:
	virtual int32 Hash(FFaerieItemStackView StackView) const override;

protected:
	// Pattern used to determine if an item qualifies as fitting this template.
	UPROPERTY(EditInstanceOnly, Category = "FISHI", meta = (DisplayThumbnail = false))
	TObjectPtr<UFaerieItemDataFilter> Pattern;

	UPROPERTY(EditAnywhere, Instanced, Category = "FISHI")
	TObjectPtr<UFaerieItemStackHashInstruction> False;

	UPROPERTY(EditAnywhere, Instanced, Category = "FISHI")
	TObjectPtr<UFaerieItemStackHashInstruction> True;
};


/**
 * Hashes a set of tokens together by their object hash.
 */
UCLASS(DisplayName = "Tokens (FISHI)")
class UFISHI_Tokens : public UFaerieItemStackHashInstruction
{
	GENERATED_BODY()

public:
	virtual int32 Hash(FFaerieItemStackView StackView) const override;

protected:
	// Pattern used to determine if an item qualifies as fitting this template.
	UPROPERTY(EditInstanceOnly, Category = "FISHI", meta = (DisplayThumbnail = false))
	TArray<TSubclassOf<UFaerieItemToken>> TokenClasses;
};